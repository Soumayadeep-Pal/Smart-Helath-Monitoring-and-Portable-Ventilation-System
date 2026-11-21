# Smart Health Monitoring System with Portable Ventilation

An end-to-end rural healthcare monitoring and emergency portable ventilation platform (**Health Mate**). The system combines Arduino sensor & ventilation firmware, a FastAPI + Machine Learning backend, and a React + Capacitor web and mobile application.

---

## Repository Structure

```text
Smart-Health-Monitoring-system-with-portable-ventilation/
├── frontend/                  # React web app + Capacitor Android & iOS wrappers
│   ├── src/                   # Pages (Landing, Login, Register, User, Doctor, Profile) & Chart.js UI
│   ├── public/                # Static assets & PWA manifest
│   ├── android/               # Capacitor Android project
│   ├── ios/                   # Capacitor iOS project
│   └── .env.example           # Frontend API endpoint template
├── backend/                   # FastAPI server, ML inference, MongoDB, Google Sheets & SMTP alerts
│   ├── python/
│   │   ├── mlModels.py        # CNN-LSTM Blood Pressure & XGBoost Stress Level inference
│   │   ├── clipText.py        # HTML email templates for registration & emergency alerts
│   │   ├── mongoOperation.py  # MongoDB Atlas CRUD for patients, doctors & devices
│   │   ├── sheetOperation.py  # Google Sheets real-time vital telemetry storage
│   │   └── smtpOperation.py   # Gmail SMTP alert & onboarding email dispatcher
│   ├── main.py                # FastAPI routes & service container
│   ├── config.py              # Environment-driven configuration
│   ├── best_model.pt          # Trained CNN-LSTM PyTorch weights
│   ├── requirements.txt       # Python dependencies
│   ├── render.yaml            # Render deployment blueprint
│   └── .env.example           # Backend environment variables template
├── firmware/                  # Refurbished Arduino sketches & archived iterations
│   ├── health_monitor/        # Combined DS18B20 + DHT11 + Servo Ambu-bag ventilation controller
│   ├── ds18b20_temperature/   # Standalone body temperature monitor with fever LED/buzzer alert
│   ├── spo2_heart_rate/       # MAX30102/MAX30105 SpO2 & BPM monitor with LCD readout
│   └── archive/               # Original development sketches and compiled .hex files
├── .gitignore
├── LICENSE
└── README.md
```

---

## System Architecture & Workflow

1. **Sensor Acquisition (`firmware/`)**:
   - **MAX30102 / MAX30105** captures heart rate (BPM) and $\text{SpO}_2$ saturation.
   - **DS18B20 & DHT11** measure patient body temperature and ambient humidity/temperature.
   - **Servo Ventilation Controller** drives an Ambu-bag mechanism with adjustable ventilation rates.
2. **Backend API & ML Pipeline (`backend/`)**:
   - **FastAPI** exposes REST endpoints (`/append/`, `/show/`, `/adduser/`, `/adddoctor/`, `/assign/`, `/patient/`, `/sendmail/`).
   - **PyTorch CNN-LSTM (`best_model.pt`)** estimates Systolic (SBP) and Diastolic (DBP) blood pressure from PPG waveforms, while **XGBoost** computes stress levels.
   - **MongoDB Atlas** manages patient, doctor, and device assignments; **Google Sheets** logs time-series vitals; **SMTP** sends registration IDs and critical emergency alerts.
3. **Web & Mobile Dashboard (`frontend/`)**:
   - Built with **React**, **Material UI**, **Chart.js**, and **Capacitor** (`com.healthmate.app`) for real-time patient vitals tracking and doctor-device management.

---

## Quick Start

### 1. Backend Setup
```bash
cd backend
cp .env.example .env   # Fill in MongoDB, SMTP, and Google Service Account details
python -m venv .venv
# Windows: .venv\Scripts\activate | macOS/Linux: source .venv/bin/activate
pip install -r requirements.txt
uvicorn main:app --reload --host 0.0.0.0 --port 8000
```

### 2. Frontend Setup
```bash
cd frontend
cp .env.example .env   # Set REACT_APP_API_KEY=http://localhost:8000
npm install
npm start
```

### 3. Firmware Upload
1. Open the desired `.ino` file inside `firmware/` using the **Arduino IDE**.
2. Install required libraries (`MAX30105`, `LiquidCrystal_I2C`, `DHT sensor library`, `OneWire`, `DallasTemperature`, `Servo`).
3. Select your target board (Arduino Uno / Nano) and COM port, then click **Upload**.

---

## License
Licensed under the Apache License 2.0 — see [LICENSE](LICENSE) for details.
