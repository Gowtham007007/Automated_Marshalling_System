🌟 **Phase 1 – Problem Definition & Environment Constraints**

**Understanding the Need for a Low-Cost Automated Marshalling System**

---

### **1. Problem Overview**

Aircraft marshalling guides an aircraft safely during parking, but the traditional human-based method has several limitations. It exposes personnel to safety risks, depends heavily on visibility, and is prone to human error and fatigue. Continuous manpower is required, and signal clarity reduces in poor weather conditions.

Large airports use advanced VDGS systems to automate this process, but these systems are extremely costly, require permanent infrastructure, and are not suitable for small airports, UAV test zones, or educational environments.

This highlights a clear technology gap that our project aims to fill.

---

### **2. Objective of Phase 1**

To clearly define the core problem, understand environmental factors, and outline the basic architecture required for a portable, affordable, sensor-based automated marshalling system.

---

### **3. Key Outcomes of Phase 1**

- Identified the need for a low-cost, mini-VDGS system that provides essential marshalling signals.

• Defined core functional requirements: alignment detection, movement guidance, and stopping indication.

• Studied environmental constraints such as lighting variations, short-range detection (20–200 cm), and power limitations.

• Finalized the initial system flow from sensing → processing → guidance output.

---

### **4. High-Level Architecture Identified**

Laser (aircraft nose) → Sensor Board (LDR/Photodiodes) → Microcontroller (STM32/ESP32) → Guidance Logic → Display/GUI.

This structure ensures simplicity, portability, and real-time operation.

---

### **5. Implementation Decisions in Phase 1**

- Use of laser-based alignment detection for simplicity and accuracy.

• Selection of LDR/photodiode sensors for left/right deviation.

• Ultrasonic/IR sensors chosen for forward distance and STOP detection.

• Microcontroller with ADC capability required for sensor readings.

• Digital display chosen to replace traditional hand signals.
