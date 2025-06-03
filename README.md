# Co-Play Wand: Interactive Gesture-Controlled Music & Visuals

This project was developed as part of the TECHIN 515 course at the University of Washington. **Co-Play Wand** is an interactive system that empowers audience members to influence live music performances using motion-based gestures. By recognizing specific arm movements via a custom hardware wand, we enable real-time control over audio effects and stage lighting, bridging the gap between audience and performer.

## 🎯 Project Goal

Transform passive concert-goers into active co-creators through intuitive physical gestures. Our system aims to amplify audience engagement and co-performance in a playful, immersive way.

## 🔧 System Overview

### Hardware
- **ESP32-WROOM** (gesture recognition and communication)
- **MPU6050** (accelerometer for motion sensing)
- **FastLED-compatible LED strip** (visual feedback)
- **Push button trigger** (start gesture capture)
- **Battery-powered wand housing**

### Software
- **Edge Impulse**: Used to train ML model to recognize 3 hand-drawn gesture paths: V, S, and O
- **Arduino code**: Runs real-time gesture inference on-device and triggers visual/audio effects
- **Python Audio Processor** (PC): Applies sound effects based on the inferred gesture (e.g. distortion, clean, lowpass)

## 🤖 Recognized Gestures

| Gesture | Description                | Effect Triggered             |
|---------|----------------------------|------------------------------|
| V       | Push forward               | Green LED + Clean sound      |
| S       | Swipe left & right         | Blue LED + Distortion effect |
| O       | Draw circle upwards        | Red LED + Lowpass filter     |

## 📦 Installation

### Arduino Side (ESP32)
1. Flash the code in `wand_firmware/` to your ESP32 using Arduino IDE
2. Install required libraries:
   - `Adafruit_MPU6050`
   - `FastLED`
   - `WiFi`, `HTTPClient` (for Firebase integration)
3. Place `a515_fiinal_inferencing.h` from Edge Impulse export in project directory

### Audio Effects Script (PC-side)
1. Navigate to `audio_effects/`
2. Install Python dependencies:
   ```bash
   pip install pyaudio numpy scipy
