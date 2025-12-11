#include "mbed.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"
#include "stm32746g_discovery_audio.h"
#include <math.h>

TS_StateTypeDef TS_State;

// ==================== AVIATION HMI COLOR SCHEME ====================
#define HMI_BACKGROUND      0xFF000814    // Dark Navy
#define HMI_SURFACE         0xFF0A1929    // Surface Panel
#define HMI_ACCENT_BLUE     0xFF00D9FF    // Cyan Accent
#define HMI_DISPLAY_GREEN   0xFF00FF41    // Aviation Green
#define HMI_CAUTION_AMBER   0xFFFFBF00    // Caution Amber
#define HMI_WARNING_RED     0xFFFF003C    // Warning Red
#define HMI_TEXT_WHITE      0xFFFFFFFF    // White
#define HMI_TEXT_GRAY       0xFF8B9DC3    // Gray Text
#define HMI_GRID_LINE       0xFF1E3A5F    // Grid Lines
#define HMI_INDICATOR_ON    0xFF00FF88    // Active Green
#define HMI_PANEL_BORDER    0xFF2D5F8D    // Panel Border

// Screen constants
#define SCREEN_W 480
#define SCREEN_H 272

// LDR threshold for ON detection
#define LDR_THRESHOLD 0.5

// ==================== HARDWARE PINS ====================
AnalogIn l1(A0), l2(A1), l3(A2), l4(A3), l5(A4), l6(A5);
Serial pc(USBTX, USBRX); 
DigitalIn ir_sensor(D8);
DigitalOut trig(D6);
DigitalIn echo(D5);

// ==================== THREAD CONTROL FLAGS ====================
volatile bool serial_thread_running = true;
volatile bool automation_active = false;
volatile float current_distance = -1.0f;
Mutex lcd_mutex;  // Mutex to protect LCD access
Mutex sensor_mutex;  // Mutex to protect sensor access

// ==================== BEEP TONES (Simulated) ====================
void play_beep(uint16_t freq, uint16_t duration) {
    // For actual sound, initialize BSP_AUDIO_OUT and use it
    // This is a placeholder for audio feedback
}

// ==================== ADVANCED DRAWING PRIMITIVES ====================

void draw_circle_outline(uint16_t x, uint16_t y, uint16_t radius, uint32_t color, uint8_t thickness) {
    for(int t = 0; t < thickness; t++) {
        BSP_LCD_SetTextColor(color);
        BSP_LCD_DrawCircle(x, y, radius - t);
    }
}

void draw_hmi_panel(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char* title) {
    // Panel background
    BSP_LCD_SetTextColor(HMI_SURFACE);
    BSP_LCD_FillRect(x, y, w, h);
    
    // Border with 3D effect
    BSP_LCD_SetTextColor(HMI_PANEL_BORDER);
    BSP_LCD_DrawRect(x, y, w, h);
    BSP_LCD_DrawRect(x + 1, y + 1, w - 2, h - 2);
    
    // Corner accents
    BSP_LCD_SetTextColor(HMI_ACCENT_BLUE);
    BSP_LCD_DrawHLine(x + 2, y + 2, 10);
    BSP_LCD_DrawVLine(x + 2, y + 2, 10);
    BSP_LCD_DrawHLine(x + w - 12, y + 2, 10);
    BSP_LCD_DrawVLine(x + w - 3, y + 2, 10);
    
    // Title bar
    if(title != NULL) {
        BSP_LCD_SetTextColor(HMI_PANEL_BORDER);
        BSP_LCD_FillRect(x + 2, y + 2, w - 4, 20);
        BSP_LCD_SetFont(&Font12);
        BSP_LCD_SetBackColor(HMI_PANEL_BORDER);
        BSP_LCD_SetTextColor(HMI_ACCENT_BLUE);
        BSP_LCD_DisplayStringAt(x + 8, y + 7, (uint8_t*)title, LEFT_MODE);
    }
}

void draw_aviation_button(uint16_t x, uint16_t y, uint16_t w, uint16_t h, 
                          const char* text, uint32_t color, bool active) {
    // Button body
    if(active) {
        BSP_LCD_SetTextColor(color);
        BSP_LCD_FillRect(x + 2, y + 2, w - 4, h - 4);
    } else {
        BSP_LCD_SetTextColor(HMI_SURFACE);
        BSP_LCD_FillRect(x + 2, y + 2, w - 4, h - 4);
    }
    
    // Border
    BSP_LCD_SetTextColor(color);
    BSP_LCD_DrawRect(x, y, w, h);
    BSP_LCD_DrawRect(x + 1, y + 1, w - 2, h - 2);
    
    // Corner indicators
    BSP_LCD_FillRect(x, y, 4, 4);
    BSP_LCD_FillRect(x + w - 4, y, 4, 4);
    BSP_LCD_FillRect(x, y + h - 4, 4, 4);
    BSP_LCD_FillRect(x + w - 4, y + h - 4, 4, 4);
    
    // Text
    BSP_LCD_SetFont(&Font16);
    if(active) {
        BSP_LCD_SetBackColor(color);
        BSP_LCD_SetTextColor(HMI_BACKGROUND);
    } else {
        BSP_LCD_SetBackColor(HMI_SURFACE);
        BSP_LCD_SetTextColor(color);
    }
    uint16_t text_x = x + (w - strlen(text) * 11) / 2;
    uint16_t text_y = y + (h - 16) / 2;
    BSP_LCD_DisplayStringAt(text_x, text_y, (uint8_t*)text, LEFT_MODE);
}

// ==================== AIRCRAFT ICON (High Quality) ====================
void draw_aircraft_icon_hq(uint16_t x, uint16_t y, uint32_t color, bool glow) {
    // Glow effect
    if(glow) {
        BSP_LCD_SetTextColor(color & 0x40FFFFFF);
        BSP_LCD_FillCircle(x, y, 45);
    }
    
    BSP_LCD_SetTextColor(color);
    
    // Fuselage (main body)
    BSP_LCD_FillRect(x - 6, y - 25, 12, 50);
    BSP_LCD_FillCircle(x, y - 25, 8);
    
    // Wings
    BSP_LCD_FillRect(x - 35, y - 5, 70, 8);
    // Wing tips
    for(int i = 0; i < 5; i++) {
        BSP_LCD_DrawHLine(x - 35 + i, y - 5 - i, 3);
        BSP_LCD_DrawHLine(x + 32 - i, y - 5 - i, 3);
    }
    
    // Tail
    BSP_LCD_FillRect(x - 15, y + 20, 30, 5);
    BSP_LCD_FillRect(x - 5, y + 15, 10, 15);
    
    // Engines
    BSP_LCD_FillCircle(x - 18, y, 5);
    BSP_LCD_FillCircle(x + 18, y, 5);
    
    // Cockpit window
    BSP_LCD_SetTextColor(HMI_ACCENT_BLUE);
    BSP_LCD_FillRect(x - 3, y - 20, 6, 8);
    
    // Details
    BSP_LCD_SetTextColor(0x80FFFFFF);
    BSP_LCD_DrawVLine(x - 1, y - 20, 40);
}

// ==================== DIRECTION ARROWS (Static - No Animation Glitches) ====================
void draw_arrow_left_hmi(uint16_t x, uint16_t y, uint32_t color) {
    BSP_LCD_SetTextColor(color);
    
    // Arrow shaft
    BSP_LCD_FillRect(x + 20, y + 15, 30, 10);
    
    // Arrow head - perfect triangle
    for(int i = 0; i < 15; i++) {
        BSP_LCD_DrawVLine(x + i, y + 20 - i, 2 * i);
    }
    
    // Clean outline
    BSP_LCD_SetTextColor(HMI_BACKGROUND);
    BSP_LCD_DrawRect(x + 20, y + 15, 30, 10);
}

void draw_arrow_right_hmi(uint16_t x, uint16_t y, uint32_t color) {
    BSP_LCD_SetTextColor(color);
    
    // Arrow shaft
    BSP_LCD_FillRect(x + 10, y + 15, 30, 10);
    
    // Arrow head - perfect triangle pointing right
    for(int i = 0; i < 15; i++) {
        BSP_LCD_DrawVLine(x + 40 + i, y + 5 + i, 30 - 2 * i);
    }
    
    // Clean outline
    BSP_LCD_SetTextColor(HMI_BACKGROUND);
    BSP_LCD_DrawRect(x + 10, y + 15, 30, 10);
}

void draw_arrow_up_hmi(uint16_t x, uint16_t y, uint32_t color) {
    BSP_LCD_SetTextColor(color);
    
    // Arrow shaft
    BSP_LCD_FillRect(x + 15, y + 20, 10, 30);
    
    // Arrow head - perfect triangle pointing up
    for(int i = 0; i < 15; i++) {
        BSP_LCD_DrawHLine(x + 20 - i, y + i, 2 * i);
    }
    
    // Clean outline
    BSP_LCD_SetTextColor(HMI_BACKGROUND);
    BSP_LCD_DrawRect(x + 15, y + 20, 10, 30);
}

// ==================== STOP SIGN (Static - No Pulsing) ====================
void draw_stop_sign_hmi(uint16_t x, uint16_t y, uint32_t color) {
    BSP_LCD_SetTextColor(color);
    
    // Octagon
    BSP_LCD_FillRect(x + 8, y, 24, 40);
    BSP_LCD_FillRect(x, y + 8, 40, 24);
    BSP_LCD_FillRect(x + 4, y + 4, 6, 6);
    BSP_LCD_FillRect(x + 30, y + 4, 6, 6);
    BSP_LCD_FillRect(x + 4, y + 30, 6, 6);
    BSP_LCD_FillRect(x + 30, y + 30, 6, 6);
    
    // Border
    BSP_LCD_SetTextColor(HMI_TEXT_WHITE);
    BSP_LCD_DrawCircle(x + 20, y + 20, 20);
    BSP_LCD_DrawCircle(x + 20, y + 20, 19);
    
    // STOP text
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_SetBackColor(color);
    BSP_LCD_SetTextColor(HMI_TEXT_WHITE);
    BSP_LCD_DisplayStringAt(x + 5, y + 16, (uint8_t*)"STOP", LEFT_MODE);
}

// ==================== STATUS INDICATORS ====================
void draw_status_led(uint16_t x, uint16_t y, uint32_t color, bool active) {
    if(active) {
        BSP_LCD_SetTextColor(color);
        BSP_LCD_FillCircle(x, y, 6);
        BSP_LCD_SetTextColor(color & 0x60FFFFFF);
        BSP_LCD_FillCircle(x, y, 9);
    } else {
        BSP_LCD_SetTextColor(0xFF1A1A1A);
        BSP_LCD_FillCircle(x, y, 6);
    }
    BSP_LCD_SetTextColor(HMI_GRID_LINE);
    draw_circle_outline(x, y, 7, HMI_GRID_LINE, 1);
}

// ==================== LCD INIT ====================
void lcd_init() {
    BSP_LCD_Init();
    BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);
    BSP_LCD_SelectLayer(0);
    BSP_LCD_Clear(HMI_BACKGROUND);
}

// ==================== HOME SCREEN WITH DISTANCE DISPLAY ====================
void show_home_screen() {
    lcd_mutex.lock();
    BSP_LCD_Clear(HMI_BACKGROUND);
    
    // Grid background
    BSP_LCD_SetTextColor(HMI_GRID_LINE);
    for(int i = 0; i < SCREEN_H; i += 20) {
        BSP_LCD_DrawHLine(0, i, SCREEN_W);
    }
    for(int i = 0; i < SCREEN_W; i += 20) {
        BSP_LCD_DrawVLine(i, 0, SCREEN_H);
    }
    
    // Top header panel
    draw_hmi_panel(10, 5, 460, 45, "AIRCRAFT GROUND CONTROL");
    
    // System info
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_SetBackColor(HMI_SURFACE);
    BSP_LCD_SetTextColor(HMI_ACCENT_BLUE);
    BSP_LCD_DisplayStringAt(20, 30, (uint8_t*)"MARSHALLING SYSTEM v3.1", LEFT_MODE);
    
    // Status LEDs
    BSP_LCD_SetFont(&Font12);
    BSP_LCD_SetTextColor(HMI_TEXT_GRAY);
    BSP_LCD_DisplayStringAt(320, 30, (uint8_t*)"SYS", LEFT_MODE);
    draw_status_led(350, 35, HMI_INDICATOR_ON, true);
    BSP_LCD_DisplayStringAt(360, 30, (uint8_t*)"PWR", LEFT_MODE);
    draw_status_led(390, 35, HMI_INDICATOR_ON, true);
    BSP_LCD_DisplayStringAt(400, 30, (uint8_t*)"RDY", LEFT_MODE);
    draw_status_led(430, 35, HMI_DISPLAY_GREEN, true);
    
    // Main display panel
    draw_hmi_panel(30, 60, 300, 140, "OPERATION CONTROL");
    
    // Aircraft icon in center
    draw_aircraft_icon_hq(180, 115, HMI_ACCENT_BLUE, true);
    
    // System status text
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_SetBackColor(HMI_SURFACE);
    BSP_LCD_SetTextColor(HMI_DISPLAY_GREEN);
    BSP_LCD_DisplayStringAt(40, 160, (uint8_t*)"SYSTEM OPERATIONAL", LEFT_MODE);
    
    BSP_LCD_SetFont(&Font12);
    BSP_LCD_SetTextColor(HMI_TEXT_GRAY);
    BSP_LCD_DisplayStringAt(40, 178, (uint8_t*)"SELECT MODE TO BEGIN", LEFT_MODE);
    
    // Distance panel (NEW)
    draw_hmi_panel(340, 60, 130, 140, "DISTANCE");
    
    // Get current distance
    float dist = current_distance;
    
    BSP_LCD_SetFont(&Font20);
    BSP_LCD_SetBackColor(HMI_SURFACE);
    
    if(dist > 0 && dist <= 400) {
        // Valid distance reading
        char dist_str[20];
        sprintf(dist_str, "%.1f", dist);
        
        uint32_t dist_color = HMI_DISPLAY_GREEN;
        if(dist < 10) dist_color = HMI_WARNING_RED;
        else if(dist < 30) dist_color = HMI_CAUTION_AMBER;
        
        BSP_LCD_SetTextColor(dist_color);
        BSP_LCD_DisplayStringAt(350, 110, (uint8_t*)dist_str, LEFT_MODE);
        
        BSP_LCD_SetFont(&Font16);
        BSP_LCD_DisplayStringAt(350, 135, (uint8_t*)"cm", LEFT_MODE);
        
        // Status indicator
        BSP_LCD_SetFont(&Font12);
        BSP_LCD_SetTextColor(HMI_TEXT_GRAY);
        if(dist < 10) {
            BSP_LCD_DisplayStringAt(350, 160, (uint8_t*)"TOO CLOSE", LEFT_MODE);
        } else if(dist < 30) {
            BSP_LCD_DisplayStringAt(350, 160, (uint8_t*)"CAUTION", LEFT_MODE);
        } else {
            BSP_LCD_DisplayStringAt(350, 160, (uint8_t*)"SAFE", LEFT_MODE);
        }
    } else {
        // Out of range
        BSP_LCD_SetTextColor(HMI_TEXT_GRAY);
        BSP_LCD_DisplayStringAt(350, 110, (uint8_t*)"---", LEFT_MODE);
        
        BSP_LCD_SetFont(&Font12);
        BSP_LCD_DisplayStringAt(350, 135, (uint8_t*)"OUT OF", LEFT_MODE);
        BSP_LCD_DisplayStringAt(350, 150, (uint8_t*)"RANGE", LEFT_MODE);
    }
    
    // Control buttons (3 buttons now)
    draw_aviation_button(35, 215, 130, 45, "AUTO MODE", HMI_DISPLAY_GREEN, false);
    draw_aviation_button(175, 215, 130, 45, "DISTANCE", HMI_ACCENT_BLUE, false);
    draw_aviation_button(315, 215, 130, 45, "EXIT", HMI_WARNING_RED, false);
    
    // Corner decorations
    BSP_LCD_SetTextColor(HMI_ACCENT_BLUE);
    for(int i = 0; i < 15; i++) {
        BSP_LCD_DrawPixel(5 + i, 5, HMI_ACCENT_BLUE);
        BSP_LCD_DrawPixel(5, 5 + i, HMI_ACCENT_BLUE);
        BSP_LCD_DrawPixel(SCREEN_W - 6 - i, 5, HMI_ACCENT_BLUE);
        BSP_LCD_DrawPixel(SCREEN_W - 6, 5 + i, HMI_ACCENT_BLUE);
    }
    lcd_mutex.unlock();
}

// ==================== DISTANCE DETAIL SCREEN ====================
void show_distance_screen() {
    while(1) {
        lcd_mutex.lock();
        BSP_LCD_Clear(HMI_BACKGROUND);
        
        // Grid background
        BSP_LCD_SetTextColor(HMI_GRID_LINE);
        for(int i = 0; i < SCREEN_H; i += 20) {
            BSP_LCD_DrawHLine(0, i, SCREEN_W);
        }
        
        // Header
        draw_hmi_panel(10, 5, 460, 45, "DISTANCE MONITORING");
        
        BSP_LCD_SetFont(&Font16);
        BSP_LCD_SetBackColor(HMI_SURFACE);
        BSP_LCD_SetTextColor(HMI_ACCENT_BLUE);
        BSP_LCD_DisplayStringAt(20, 30, (uint8_t*)"ULTRASONIC SENSOR", LEFT_MODE);
        
        // Status LEDs
        BSP_LCD_SetFont(&Font12);
        BSP_LCD_SetTextColor(HMI_TEXT_GRAY);
        draw_status_led(350, 23, HMI_INDICATOR_ON, true);
        draw_status_led(370, 23, HMI_DISPLAY_GREEN, true);
        draw_status_led(390, 23, HMI_ACCENT_BLUE, true);
        
        // Main panel
        draw_hmi_panel(30, 60, 420, 150, "DISTANCE READING");
        
        float dist = current_distance;
        
        BSP_LCD_SetFont(&Font24);
        BSP_LCD_SetBackColor(HMI_SURFACE);
        
        if(dist > 0 && dist <= 400) {
            char dist_str[30];
            sprintf(dist_str, "%.2f cm", dist);
            
            uint32_t dist_color = HMI_DISPLAY_GREEN;
            if(dist < 10) dist_color = HMI_WARNING_RED;
            else if(dist < 30) dist_color = HMI_CAUTION_AMBER;
            
            BSP_LCD_SetTextColor(dist_color);
            BSP_LCD_DisplayStringAt(0, 110, (uint8_t*)dist_str, CENTER_MODE);
            
            // Status bar
            BSP_LCD_SetFont(&Font16);
            if(dist < 10) {
                BSP_LCD_SetTextColor(HMI_WARNING_RED);
                BSP_LCD_DisplayStringAt(0, 145, (uint8_t*)"CRITICAL - TOO CLOSE", CENTER_MODE);
            } else if(dist < 30) {
                BSP_LCD_SetTextColor(HMI_CAUTION_AMBER);
                BSP_LCD_DisplayStringAt(0, 145, (uint8_t*)"CAUTION - PROXIMITY ALERT", CENTER_MODE);
            } else if(dist < 100) {
                BSP_LCD_SetTextColor(HMI_DISPLAY_GREEN);
                BSP_LCD_DisplayStringAt(0, 145, (uint8_t*)"SAFE DISTANCE", CENTER_MODE);
            } else {
                BSP_LCD_SetTextColor(HMI_ACCENT_BLUE);
                BSP_LCD_DisplayStringAt(0, 145, (uint8_t*)"CLEAR - NO OBSTACLES", CENTER_MODE);
            }
            
            // Visual bar indicator
            BSP_LCD_SetTextColor(HMI_GRID_LINE);
            BSP_LCD_DrawRect(50, 175, 380, 20);
            
            int bar_length = (int)((dist / 400.0f) * 376);
            if(bar_length > 376) bar_length = 376;
            
            BSP_LCD_SetTextColor(dist_color);
            if(bar_length > 0) {
                BSP_LCD_FillRect(52, 177, bar_length, 16);
            }
            
        } else {
            BSP_LCD_SetTextColor(HMI_TEXT_GRAY);
            BSP_LCD_DisplayStringAt(0, 110, (uint8_t*)"OUT OF RANGE", CENTER_MODE);
            
            BSP_LCD_SetFont(&Font16);
            BSP_LCD_DisplayStringAt(0, 145, (uint8_t*)"NO VALID READING", CENTER_MODE);
        }
        
        // Footer
        BSP_LCD_SetFont(&Font12);
        BSP_LCD_SetBackColor(HMI_BACKGROUND);
        BSP_LCD_SetTextColor(HMI_TEXT_GRAY);
        BSP_LCD_DisplayStringAt(0, 225, (uint8_t*)"TAP SCREEN TO RETURN TO MAIN MENU", CENTER_MODE);
        
        lcd_mutex.unlock();
        
        // Check for touch to exit
        BSP_TS_GetState(&TS_State);
        if(TS_State.touchDetected) {
            play_beep(1000, 50);
            ThisThread::sleep_for(300);
            return;
        }
        
        ThisThread::sleep_for(100);  // Update 10 times per second
    }
}

// ==================== TOUCH CHECK (UPDATED FOR 3 BUTTONS) ====================
int check_touch() {
    BSP_TS_GetState(&TS_State);
    if (!TS_State.touchDetected) return 0;

    int x = TS_State.touchX[0];
    int y = TS_State.touchY[0];

    // Auto Mode button
    if (x > 35 && x < 165 && y > 215 && y < 260) {
        play_beep(1000, 50);
        return 1;
    }
    // Distance button
    if (x > 175 && x < 305 && y > 215 && y < 260) {
        play_beep(1000, 50);
        return 2;
    }
    // Exit button
    if (x > 315 && x < 445 && y > 215 && y < 260) {
        play_beep(800, 50);
        return 3;
    }
    return 0;
}

// ==================== AUTOMATION SCREENS (SENSOR-DRIVEN) ====================
void draw_automation_screen(const char* title, const char* instruction, 
                           uint32_t color, uint8_t mode, uint8_t frame, bool redraw_all) {
    
    lcd_mutex.lock();
    
    // Full redraw only on state change
    if(redraw_all) {
        BSP_LCD_Clear(HMI_BACKGROUND);
        
        // Grid background
        BSP_LCD_SetTextColor(HMI_GRID_LINE);
        for(int i = 0; i < SCREEN_H; i += 20) {
            BSP_LCD_DrawHLine(0, i, SCREEN_W);
        }
        
        // Header
        draw_hmi_panel(10, 5, 460, 35, "ACTIVE MARSHALLING");
        BSP_LCD_SetFont(&Font12);
        BSP_LCD_SetBackColor(HMI_SURFACE);
        BSP_LCD_SetTextColor(HMI_ACCENT_BLUE);
        BSP_LCD_DisplayStringAt(20, 20, (uint8_t*)"MODE: SENSOR-DRIVEN", LEFT_MODE);
        
        // Main instruction panel
        draw_hmi_panel(20, 50, 440, 160, "MARSHALLING DIRECTIVE");
        
        // Title
        BSP_LCD_SetFont(&Font24);
        BSP_LCD_SetBackColor(HMI_SURFACE);
        BSP_LCD_SetTextColor(color);
        BSP_LCD_DisplayStringAt(0, 90, (uint8_t*)title, CENTER_MODE);
        
        // Instruction
        BSP_LCD_SetFont(&Font16);
        BSP_LCD_SetTextColor(HMI_TEXT_GRAY);
        BSP_LCD_DisplayStringAt(0, 118, (uint8_t*)instruction, CENTER_MODE);
        
        // Draw static direction indicators
        if(mode == 0) { // LEFT
            draw_arrow_left_hmi(140, 145, color);
            draw_arrow_left_hmi(240, 145, color);
            draw_arrow_left_hmi(340, 145, color);
        }
        else if(mode == 1) { // RIGHT
            draw_arrow_right_hmi(80, 145, color);
            draw_arrow_right_hmi(180, 145, color);
            draw_arrow_right_hmi(280, 145, color);
        }
        else if(mode == 2) { // STRAIGHT
            draw_arrow_up_hmi(130, 145, color);
            draw_arrow_up_hmi(215, 145, color);
            draw_arrow_up_hmi(300, 145, color);
        }
        else if(mode == 3) { // STOP
            draw_stop_sign_hmi(140, 150, color);
            draw_stop_sign_hmi(220, 150, color);
            draw_stop_sign_hmi(300, 150, color);
        }
        
        // Footer
        BSP_LCD_SetFont(&Font12);
        BSP_LCD_SetBackColor(HMI_BACKGROUND);
        BSP_LCD_SetTextColor(HMI_TEXT_GRAY);
        BSP_LCD_DisplayStringAt(0, 225, (uint8_t*)"TAP SCREEN TO ABORT SEQUENCE", CENTER_MODE);
        
        // Progress bar outline
        BSP_LCD_SetTextColor(HMI_GRID_LINE);
        BSP_LCD_DrawRect(100, 245, 280, 12);
    }
    
    // Update only dynamic elements
    // Status indicators
    draw_status_led(350, 23, color, true);
    draw_status_led(370, 23, color, (frame % 4) < 2);
    draw_status_led(390, 23, color, (frame % 6) < 3);
    
    // Animated status bar
    BSP_LCD_SetTextColor(HMI_SURFACE);
    BSP_LCD_FillRect(25, 73, 400, 3);
    BSP_LCD_SetTextColor(color);
    int bar_width = (frame % 100) * 4;
    if(bar_width > 0) {
        BSP_LCD_FillRect(25, 73, bar_width, 3);
    }
    
    // Progress bar
    BSP_LCD_SetTextColor(HMI_BACKGROUND);
    BSP_LCD_FillRect(102, 247, 276, 8);
    BSP_LCD_SetTextColor(color);
    int progress = ((frame / 2) % 50) * 5;
    if(progress > 0) {
        BSP_LCD_FillRect(102, 247, progress, 8);
    }
    
    lcd_mutex.unlock();
}

// ==================== SENSOR-DRIVEN AUTOMATION ====================
uint8_t determine_direction_from_sensors() {
    sensor_mutex.lock();
    
    // Read LDR sensors
    float a0 = l1.read();
    float a1 = l2.read();
    float a2 = l3.read();
    float a3 = l4.read();
    float a4 = l5.read();
    
    // Read IR sensor
    int ir = ir_sensor.read();
    
    sensor_mutex.unlock();
    
    // Priority: IR sensor (STOP) > Turn signals > Straight
    
    // If IR sensor detects obstacle (active low), STOP
    if(ir == 0) {
        return 3;  // STOP
    }
    
    // If A0 OR A1 are ON (above threshold), TURN RIGHT
    if(a0 > LDR_THRESHOLD || a1 > LDR_THRESHOLD) {
        return 1;  // TURN RIGHT
    }
    
    // If A3 OR A4 are ON, TURN LEFT
    if(a3 > LDR_THRESHOLD || a4 > LDR_THRESHOLD) {
        return 0;  // TURN LEFT
    }
    
    // If A2 is ON, PROCEED STRAIGHT
    if(a2 > LDR_THRESHOLD) {
        return 2;  // STRAIGHT
    }
    
    // Default: PROCEED STRAIGHT (no sensors active)
    return 2;  // STRAIGHT
}

void run_automation() {
    uint8_t frame = 0;
    uint8_t state = 255;
    uint8_t prev_state = 254;  // Force initial redraw
    
    automation_active = true;
    
    const char* titles[] = {"TURN LEFT", "TURN RIGHT", "PROCEED STRAIGHT", "STOP AIRCRAFT"};
    const char* instructions[] = {
        "AIRCRAFT TURN PORT SIDE",
        "AIRCRAFT TURN STARBOARD",
        "CONTINUE FORWARD TAXI",
        "HALT - OBSTACLE DETECTED"
    };
    uint32_t colors[] = {HMI_CAUTION_AMBER, HMI_CAUTION_AMBER, HMI_DISPLAY_GREEN, HMI_WARNING_RED};
    
    pc.printf("\r\n========== AUTO MODE STARTED ==========\r\n");
    pc.printf("System is now sensor-driven.\r\n");
    pc.printf("LDR sensors controlling direction.\r\n");
    pc.printf("IR sensor controlling STOP.\r\n\r\n");
    
    while(1) {
        // Get direction from sensors
        state = determine_direction_from_sensors();
        
        // Only full redraw when state changes
        bool redraw_all = (state != prev_state);
        
        if(redraw_all) {
            pc.printf("Direction changed to: %s\r\n", titles[state]);
            play_beep(1200, 30);
        }
        
        draw_automation_screen(titles[state], instructions[state], colors[state], state, frame, redraw_all);
        prev_state = state;
        
        // Check for touch to abort
        BSP_TS_GetState(&TS_State);
        if(TS_State.touchDetected) {
            play_beep(500, 100);
            automation_active = false;
            pc.printf("\r\n========== AUTO MODE ABORTED ==========\r\n\r\n");
            return;
        }
        
        frame++;
        
        ThisThread::sleep_for(100);  // Check sensors 10 times per second
    }
}

// ==================== ULTRASONIC DISTANCE MEASUREMENT (SAFE) ====================
float read_ultrasonic_distance() {
    sensor_mutex.lock();
    
    // Send trigger pulse
    trig = 0;
    wait_us(2);
    trig = 1;
    wait_us(10);
    trig = 0;
    
    // Wait for echo start with timeout (max 30ms)
    Timer timeout;
    timeout.start();
    while (echo.read() == 0) {
        if(timeout.read_ms() > 30) {
            sensor_mutex.unlock();
            return -1.0f;  // Timeout - no echo
        }
    }
    
    // Measure high pulse duration with timeout
    Timer pulse_timer;
    pulse_timer.start();
    timeout.reset();
    
    while (echo.read() == 1) {
        if(timeout.read_ms() > 30) {
            sensor_mutex.unlock();
            return -1.0f;  // Timeout - echo too long
        }
    }
    pulse_timer.stop();
    
    float pulse_us = pulse_timer.read_us();
    float distance_cm = pulse_us * 0.01715f;
    
    sensor_mutex.unlock();
    
    // Validate distance (HC-SR04 range: 2cm - 400cm)
    if(distance_cm < 2.0f || distance_cm > 400.0f) {
        return -1.0f;  // Out of range
    }
    
    return distance_cm;
}

// ==================== SERIAL MONITOR THREAD ====================
void serial_monitor_thread() {
    while(serial_thread_running) {
        sensor_mutex.lock();
        
        // Read LDR sensors
        float a = l1.read();
        float b = l2.read();
        float c = l3.read();
        float d = l4.read();
        float e = l5.read();
        float f = l6.read();
        
        // Read IR sensor
        int ir_value = ir_sensor.read();
        
        sensor_mutex.unlock();
        
        const char* ir_status = (ir_value == 0) ? "Obstacle Detected" : "Clear";
        
        // Read Ultrasonic sensor
        float distance_cm = read_ultrasonic_distance();
        current_distance = distance_cm;  // Update global distance
        
        // Print all sensor data
        pc.printf("\r\n========== SENSOR DATA ==========\r\n");
        pc.printf("LDR Sensors:\r\n");
        pc.printf("  A0 (1st): %.2f %s\r\n", a, (a > LDR_THRESHOLD) ? "[ON]" : "");
        pc.printf("  A1 (2nd): %.2f %s\r\n", b, (b > LDR_THRESHOLD) ? "[ON]" : "");
        pc.printf("  A2 (3rd): %.2f %s\r\n", c, (c > LDR_THRESHOLD) ? "[ON]" : "");
        pc.printf("  A3 (4th): %.2f %s\r\n", d, (d > LDR_THRESHOLD) ? "[ON]" : "");
        pc.printf("  A4 (5th): %.2f %s\r\n", e, (e > LDR_THRESHOLD) ? "[ON]" : "");
        pc.printf("  A5 (6th): %.2f\r\n", f);
        pc.printf("IR Sensor: %s\r\n", ir_status);
        
        if(distance_cm > 0) {
            pc.printf("Ultrasonic: %.2f cm", distance_cm);
            if(distance_cm < 10) pc.printf(" [CRITICAL]");
            else if(distance_cm < 30) pc.printf(" [CAUTION]");
            else if(distance_cm < 100) pc.printf(" [SAFE]");
            else pc.printf(" [CLEAR]");
            pc.printf("\r\n");
        } else {
            pc.printf("Ultrasonic: Out of range/Error\r\n");
        }
        
        // Show current direction logic
        if(automation_active) {
            pc.printf("\nActive Direction: ");
            if(ir_value == 0) {
                pc.printf("STOP (IR Sensor)\r\n");
            } else if(a > LDR_THRESHOLD || b > LDR_THRESHOLD) {
                pc.printf("TURN RIGHT (A0/A1)\r\n");
            } else if(d > LDR_THRESHOLD || e > LDR_THRESHOLD) {
                pc.printf("TURN LEFT (A3/A4)\r\n");
            } else if(c > LDR_THRESHOLD) {
                pc.printf("PROCEED STRAIGHT (A2)\r\n");
            } else {
                pc.printf("PROCEED STRAIGHT (Default)\r\n");
            }
        }
        
        pc.printf("=================================\r\n\r\n");
        
        // Wait 2 seconds between readings
        ThisThread::sleep_for(2000);
    }
}

// ==================== MAIN ====================
int main() {
    // Initialize hardware
    lcd_init();
    BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
    pc.baud(115200);
    
    // Configure IR sensor with pull-up
    ir_sensor.mode(PullUp);
    
    // Initialize ultrasonic sensor pins
    trig = 0;
    echo.mode(PullNone);
    
    // Print startup message
    pc.printf("\r\n\r\n");
    pc.printf("========================================\r\n");
    pc.printf("  AIRCRAFT MARSHALLING SYSTEM v3.1\r\n");
    pc.printf("    SENSOR-INTEGRATED VERSION\r\n");
    pc.printf("========================================\r\n");
    pc.printf("System initialized successfully.\r\n");
    pc.printf("Sensor monitoring started.\r\n");
    pc.printf("\r\nSensor Configuration:\r\n");
    pc.printf("  A0/A1: Turn Right\r\n");
    pc.printf("  A2:    Proceed Straight\r\n");
    pc.printf("  A3/A4: Turn Left\r\n");
    pc.printf("  D8(IR): Emergency Stop\r\n");
    pc.printf("  D6/D5:  Distance Sensor\r\n");
    pc.printf("\r\nLDR Threshold: %.2f\r\n\r\n", LDR_THRESHOLD);
    
    // Create and start serial monitor thread
    Thread serial_thread(osPriorityNormal, 4096);
    serial_thread.start(serial_monitor_thread);
    
    // Small delay to let serial thread start
    ThisThread::sleep_for(100);
    
    while(1) {
        show_home_screen();
        
        ThisThread::sleep_for(200);
        
        int t = 0;
        while(t == 0) {
            ThisThread::sleep_for(50);
            t = check_touch();
        }
        
        ThisThread::sleep_for(200);
        
        if(t == 1) {
            // Start sensor-driven automation mode
            pc.printf("\r\n>>> AUTO MODE SELECTED <<<\r\n");
            run_automation();
        }
        else if(t == 2) {
            // Show distance detail screen
            pc.printf("\r\n>>> DISTANCE MONITOR SELECTED <<<\r\n");
            show_distance_screen();
        }
        else if(t == 3) {
            // System exit
            lcd_mutex.lock();
            BSP_LCD_Clear(HMI_BACKGROUND);
            draw_hmi_panel(90, 80, 300, 110, "SYSTEM SHUTDOWN");
            draw_aircraft_icon_hq(240, 135, HMI_WARNING_RED, true);
            BSP_LCD_SetFont(&Font20);
            BSP_LCD_SetBackColor(HMI_SURFACE);
            BSP_LCD_SetTextColor(HMI_WARNING_RED);
            BSP_LCD_DisplayStringAt(0, 165, (uint8_t*)"SYSTEM EXITED", CENTER_MODE);
            BSP_LCD_SetFont(&Font12);
            BSP_LCD_SetTextColor(HMI_TEXT_GRAY);
            BSP_LCD_DisplayStringAt(0, 185, (uint8_t*)"SAFE TO POWER DOWN", CENTER_MODE);
            lcd_mutex.unlock();
            
            play_beep(800, 200);
            
            pc.printf("\r\n========================================\r\n");
            pc.printf("  SYSTEM SHUTDOWN INITIATED\r\n");
            pc.printf("========================================\r\n");
            
            // Stop serial thread gracefully
            serial_thread_running = false;
            serial_thread.join();
            
            pc.printf("Sensor monitoring stopped.\r\n");
            pc.printf("Safe to power down.\r\n\r\n");
            
            ThisThread::sleep_for(2000);
            break;  // Exit main loop
            
        }
    }
}
