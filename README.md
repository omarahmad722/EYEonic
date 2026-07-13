# 👁️ EYEonic – AI-Powered Gaze-Controlled Robotic Car

> An AI-powered robotic car that enables hands-free mobility by translating eye movements into real-time navigation commands using Computer Vision, MediaPipe, OpenCV, and ESP32.

<img width="890" height="511" alt="Screenshot 2026-07-11 104935" src="https://github.com/user-attachments/assets/11450458-3754-45e1-8496-41c4c41b7b91" />


---

## 📖 Overview

EYEonic is our **A+ Graduation Project**, developed to explore how Artificial Intelligence and Computer Vision can improve accessibility for people with mobility impairments.

The system tracks the user's eye movements using a webcam and converts them into navigation commands that are sent wirelessly to an ESP32-controlled robotic car, enabling hands-free movement.

This project combines software and hardware to create an intelligent, affordable, and non-invasive mobility solution.

---

## 🎯 Problem Statement

Millions of people with mobility impairments rely on traditional control methods that may not always be practical or accessible.

Our goal was to design an affordable system capable of controlling a robotic vehicle using only eye movements.

---

## 💡 Solution

The system captures live video from a webcam and detects facial landmarks using **MediaPipe Face Mesh**.

Eye movements are analyzed using **OpenCV**, converted into movement commands, and transmitted to an **ESP32** via serial communication.

The ESP32 then controls the motors through an **L298N Motor Driver**, allowing the robotic car to move in real time.

## 🧠 System Logic

The EYEonic system follows a real-time processing pipeline that converts eye movements into robotic navigation commands.

<img width="1536" height="1024" alt="ChatGPT Image Jun 21, 2026, 01_30_14 AM (1)" src="https://github.com/user-attachments/assets/ea584a06-9676-4216-9abd-c4450017a5ed" />

## 🚀 Features

- 👁️ Real-time Eye Tracking
- 🤖 Gaze-Based Vehicle Control
- ⚡ Low-Latency Communication
- 📷 Computer Vision-Based Navigation
- 📡 ESP32 Integration
- 🛑 Obstacle Detection
- 💰 Low-Cost Hardware Design

---

## 🏗️ System Architecture

```
Webcam
   │
   ▼
OpenCV + MediaPipe
   │
Eye Tracking
   │
Movement Decision
   │
Serial Communication
   │
ESP32
   │
Motor Driver (L298N)
   │
Robotic Car
```

## 🔄 Workflow
1. Capture live video using the webcam.
2. Detect facial landmarks with MediaPipe.
3. Track eye movements.
4. Determine gaze direction.
5. Convert gaze into movement commands.
6. Send commands to ESP32.
7. Control the robotic car.
<img width="1206" height="673" alt="Screenshot 2026-07-13 041724" src="https://github.com/user-attachments/assets/af161260-5ab1-47d1-831c-0d4b6cd4cccd" />

---
## 🛡️ Safety Features

To ensure safe and reliable operation, several safety mechanisms were integrated into the system.

### Implemented Safety Measures


https://github.com/user-attachments/assets/70816323-dcf1-4815-808b-687eef104af1
  <img width="1536" height="1024" alt="WhatsApp Image 2026-06-21 at 5 09 24 AM" src="https://github.com/user-attachments/assets/57e7a0a4-fec9-4d36-a99f-d57ebe6c215d" />
<img width="740" height="1600" alt="WhatsApp Image 2026-06-21 at 5 41 35 AM" src="https://github.com/user-attachments/assets/a698d408-1c74-4ed0-941f-ab079f86d914" />
<img width="1536" height="1024" alt="ChatGPT Image Jun 21, 2026, 04_16_23 AM (1)" src="https://github.com/user-attachments/assets/b09f4425-8195-4af1-afb3-c4ba22c7032e" />


- 🛑 Automatic stop when no valid gaze is detected.
- ⚠️ Collision prevention by stopping the motors when an obstacle is too close.
- 👁️ Continuous eye-tracking validation before executing commands.
- 🔄 Stable command transmission between software and ESP32.
- ⚡ Low-latency processing for responsive control.

These safety mechanisms improve system reliability and reduce the risk of unintended movements.

## 🛠️ Technologies Used

### Programming Languages

- Python
- C++ (Arduino)

### Libraries

- OpenCV
- MediaPipe
- NumPy
- PySerial

### Hardware

- ESP32
- L298N Motor Driver
- DC Motors
- Servo Motor
- Ultrasonic Sensor
- Webcam
- 18650 Batteries

### Tools

- Arduino IDE
- Visual Studio Code
- Git & GitHub

---

## 📁 Project Structure

```
EYEonic/
│
├── docs/
├── hardware/
├── images/
├── software/
├── videos/
├── README.md
└── LICENSE
```

---

## ⚙️ Installation

### Clone the repository

```bash
git clone https://github.com/USERNAME/EYEonic-Gaze-Controlled-Robotic-Car.git
```

### Install dependencies

```bash
pip install -r requirements.txt
```

### Run

```bash
python main.py
```

---

## ⚠️ Challenges

Throughout this project, we encountered several technical challenges, including:

- Improving eye-tracking accuracy under different lighting conditions.
- Integrating software with embedded hardware.
- Reducing communication latency.
- Stabilizing gaze detection.
- Optimizing real-time performance.

Each challenge became an opportunity to improve both the system and our engineering skills.

---

## 📊 Results

✅ Successfully developed a working gaze-controlled robotic car.

✅ Achieved reliable real-time eye tracking.

✅ Wireless communication between software and hardware.

✅ Awarded **A+** as the Graduation Project.

---

## 📷 Project Gallery

### Hardware Setup

<img width="929" height="520" alt="Screenshot 2026-07-13 054113" src="https://github.com/user-attachments/assets/431d2eca-f130-4553-8aa1-8c9e70ba057c" />

## 🔌 Circuit Diagram

The following diagram illustrates the hardware wiring and connections between the ESP32, motor driver, sensors, and robotic car components.
<img width="3000" height="1800" alt="EYEonic_Power_Architecture_Updated (1)" src="https://github.com/user-attachments/assets/05d7a184-dcd0-44c2-a490-b68bacaf994c" />

---

## 🎥 Demo

Watch the project in action:

https://github.com/user-attachments/assets/8f40f9e7-3efa-4b49-873c-2f8d6b8d99fa

👉 **Demo Video:** 

---

## 📄 Documentation

- Graduation Presentation
- Project Report
- System Architecture

---

## 👥 Team

- **Rawda Hesham** – Embedded Systems & Hardware Development & presentation 
- **omar ahmed** –  Computer Vision & Documentation & Hardware Development

---

## 🌱 Lessons Learned

This project strengthened our technical and professional skills in:

- Problem Solving
- Computer Vision
- Artificial Intelligence
- Embedded Systems
- Debugging
- Teamwork
- Communication
- Critical Thinking

Beyond building a prototype, this journey taught us how to collaborate, adapt, and continuously improve through every challenge.

---

## 🔮 Future Improvements

- Deploy a deep learning-based gaze estimation model.
- Improve tracking robustness under different environments.
- Develop a mobile application.
- Add voice assistance.
- Enhance obstacle avoidance.
- Support wheelchair integration.

---

## 🤝 Acknowledgments

Special thanks to everyone who supported us throughout this journey and contributed to making this project a reality.

---

## ⭐ If you found this project interesting, consider giving it a Star!

