🌟 **Phase 2 – Hardware and Firmware Architecture**

**Designing the Physical System and Defining the Core Processing Flow**

---

### **1. Phase Objective**

The goal of Phase 2 was to design the complete hardware architecture and define how the firmware would process sensor inputs to generate accurate marshalling signals. This phase ensured that the sensing elements, controller, and output display worked together as a cohesive system.

---

### **2. Hardware Architecture Overview**

The system hardware was structured around three main blocks:

- **Alignment Detection Module:**

Consisting of a laser mounted on the aircraft model and an LDR/photodiode sensor array on the display board. This setup detects whether the aircraft is perfectly aligned or deviating left or right.

- **Proximity Measurement Module:**

An ultrasonic or IR sensor placed ahead of the display detects the aircraft’s forward movement and triggers a STOP signal when the predefined safe distance is reached.

- **Processing and Output Module:**

An STM32/ESP32 microcontroller reads sensor values, interprets them using threshold logic, and updates the TFT/LED display with marshalling signals such as LEFT, RIGHT, FORWARD, SLOW, and STOP.

---

### **3. Firmware Architecture**

The firmware design followed a simple and efficient flow:

1. Read alignment values from LDR sensors.
2. Measure distance continuously using the ultrasonic/IR sensor.
3. Apply threshold logic to determine deviation and stopping conditions.
4. Generate the correct marshalling command.
5. Update the display instantly for real-time guidance.

This ensures smooth, autonomous operation with minimal latency.

---

### **4. Key Decisions Taken**

- Chose microcontrollers with ADC support for accurate LDR readings.

• Defined pin assignments for sensors and display modules.

• Selected threshold values for left/right deviation.

• Established safe stopping distance for reliable braking indication.

• Prioritized low power consumption and modular wiring for portability.

---

### **5. Outcome of Phase 2**

By the end of this phase, the complete hardware layout and firmware flow were finalized. The system was ready for sensor integration and prototype development in Phase 3.
