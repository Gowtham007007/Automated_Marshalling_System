<div align="center">

![Automated Marshalling](https://img.shields.io/badge/Automated-Marshalling-blueviolet?style=for-the-badge)
![Mini VDGS](https://img.shields.io/badge/Mini-VDGS-teal?style=for-the-badge)
![Sensor Fusion](https://img.shields.io/badge/Sensor-Fusion-yellow?style=for-the-badge)
![Embedded Systems](https://img.shields.io/badge/Embedded-Systems-green?style=for-the-badge)
![Aviation Safety](https://img.shields.io/badge/Aviation-Safety-red?style=for-the-badge)
![Hackathon Project](https://img.shields.io/badge/Hackathon-Project-orange?style=for-the-badge)

# ✈️🌟 Welcome to the **Automated Aircraft Marshalling System** Repository 🚀  
### A Low-Cost, Portable, Mini-VDGS Designed for Smart Aviation Guidance

</div>

---

# **📑 Table of Contents**

1. ✈️ Problem Description
2. 🎯 Need & Opportunity
3. 🧠 System Concept
4. 🛠️ Solution Design
5. ⚙️ Hardware & Sensor Logic
6. 🧪 Testing Approach
7. 🌟 Key Advantages
8. 🚀 Final Outcome

---

# **✈️ 1. Problem Description**

Aircraft marshalling is a crucial ground operation that guides an aircraft from the taxiway to its exact parking stand. Traditionally, this task is performed by **human marshallers** using hand signals. However, this method comes with several challenges:

- ⚠️ **Safety Risks:** Marshallers stand dangerously close to engines, propellers, and moving aircraft.
- 😵 **Human Error & Fatigue:** Miscommunication can lead to incorrect alignment or unsafe stopping distances.
- 🌧️ **Visibility Limitations:** Weather conditions like fog, rain, and glare reduce signal clarity.
- 👥 **Operational Dependence:** Requires skilled manpower for every arrival.

Many major airports use **VDGS (Visual Docking Guidance Systems)** to solve these issues—but VDGS units are 🟥 **too expensive**, 🟥 **infrastructure-heavy**, and 🟥 **unsuitable for small airports, UAV zones, or academic labs**.

---

# **🎯 2. Need & Opportunity**

There is a clear need for a **portable, affordable, and quick-to-deploy automated marshalling solution** that can deliver the essential functions of VDGS without its cost and complexity.

Such a system would benefit:

- 🛩️ Small and regional airports
- 🎓 Training institutes
- 🧪 Research labs & UAV testing zones
- 🏫 Engineering demonstrations & hackathons

This creates an opportunity to develop a **“Mini-VDGS”**:

👉 A compact, low-power, sensor-based guidance system that automates alignment and stopping.

---

# **🧠 3. System Concept**

The core idea is simple:

> Replace the human marshaller with a digital board that senses the aircraft’s alignment and distance, then displays real-time guidance.
> 

The aircraft (or mock model) carries a **laser**, and the board contains **light sensors**, allowing the system to understand misalignment.

Sensors + Controller = Automated Guidance

Guidance + Display = Digital Marshalling

---

# **🛠️ 4. Solution Design**

The system consists of three interconnected modules:

### **🔵 4.1 Detection Module**

- 🔦 **Laser emitter** mounted on aircraft nose
- 📡 **LDR / photodiode array** detects left/right deviation
- 📏 **Ultrasonic / IR sensor** measures forward stopping distance

### **🟢 4.2 Processing Module**

- 🧠 STM32 / ESP32 controller
- Converts raw sensor data → alignment logic
- Runs threshold-based deviation detection
- Determines correct marshalling signal

### **🟣 4.3 Display & UI Module**

- 🖥️ TFT or LED display
- Shows real-time messages:
    - ⬅️ Move Left
    - ➡️ Move Right
    - ⏩ Move Forward
    - 🐢 Slow Down
    - 🛑 STOP

A GUI version acts as a simulator for pilot training.

---

# **⚙️ 5. Hardware & Sensor Logic**

| Function | Component | Logic |
| --- | --- | --- |
| Alignment | Laser + LDRs | Center hit = correct; left/right shift = misalignment |
| Approach | Ultrasonic | Measures distance in real time |
| Stop Detection | IR/Ultrasonic | Triggers STOP when threshold reached |
| Processing | STM32/ESP32 | Decision-making + display control |
| Display | TFT/LED | Converts logic → visual marshalling signal |

---

# **🧪 6. Testing Approach**

Testing was performed in phases:

- 🎯 Alignment deviation tests
- 📏 Distance calibration between 20–200 cm
- 💡 Lighting condition tests (bright light, indoor, low light)
- 🧭 Signal response time checks
- 👀 Display clarity validation

---

# **🌟 7. Key Advantages**

- 💰 **Ultra-low cost** (less than 1% of VDGS cost)
- 🎒 **Portable and easy to deploy**
- ⚡ **Low power consumption**
- 🧩 **Modular and scalable**
- 🎓 **Ideal for education, prototyping, UAV zones**
- 🚀 **A unique and innovative alternative to VDGS**

---

# **🚀 8. Final Outcome**

The final prototype successfully demonstrates automated marshalling with:

- ✔ Real-time alignment tracking
- ✔ Accurate forward distance measurement
- ✔ Clear, responsive guidance signals
- ✔ A training-friendly GUI
- ✔ A fully integrated, portable mini-VDGS design

This prototype fills a real-world technology gap by making automated marshalling **accessible, affordable, and practical** for small-scale aviation environments.
