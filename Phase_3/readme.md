🌟 **Phase 3 – Sensor & Actuator Integrated Prototype**

**Bringing the System to Life Through Real Hardware Integration**

---

### **1. 🎯 Phase Objective**

The goal of Phase 3 was to assemble all selected sensors, components, and the microcontroller into a working prototype. This phase transformed the design from a theoretical architecture into a functional physical system capable of detecting alignment and distance in real time.

---

### **2. 🔧 Sensor Integration**

- **Laser + LDR Alignment Module:**

The laser mounted on the aircraft model successfully projected onto the LDR/photodiode array.

The movement of the aircraft caused predictable shifts in sensor readings, enabling reliable LEFT and RIGHT deviation detection.

- **Ultrasonic / IR Proximity Module:**

Forward movement was captured using an ultrasonic/IR sensor.

As the aircraft approached, distance values changed smoothly, allowing accurate STOP detection at the predefined threshold.

---

### **3. 🧠 Microcontroller Processing**

The STM32/ESP32 microcontroller continuously read sensor data and applied the threshold logic defined in Phase 2.

This enabled real-time decision-making, resulting in instant marshalling signals displayed on the screen.

Firmware successfully handled:

- Alignment deviation
- Forward progression
- Safety stop
- Signal switching without lag

---

### **4. 🖥️ Display & Output Integration**

A TFT/LED screen was connected to visualize marshalling instructions.

Depending on sensor inputs, the system displayed:

⬅️ **Move Left**

➡️ **Move Right**

⏩ **Move Forward**

🐢 **Slow Down**

🛑 **STOP**

The transitions were smooth, readable, and aligned with real-time sensor updates.

---

### **5. 🧪 Calibration & Testing During Integration**

- Initial calibration of LDR sensitivity under indoor and bright-light conditions.

• Distance threshold tuning to ensure timely STOP response.

• Verified stable output with repeated movements of the aircraft model.

• Ensured minimal noise or false triggering.

---

### **6. 🚀 Outcome of Phase 3**

By the end of Phase 3, a fully functional integrated prototype was achieved.

Sensors, microcontroller logic, and visual output all worked together seamlessly, proving the concept and preparing the system for autonomous operation in Phase 4.
