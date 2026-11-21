import logging
import random
from datetime import datetime
from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel

from config import settings
from python.clipText import alertText, gretingSystem
from python.mlModels import mlModel
from python.mongoOperation import MongoConnection
from python.sheetOperation import GspreadConnection
from python.smtpOperation import SMTPserver

logging.basicConfig(level=logging.INFO, format="%(asctime)s [%(levelname)s] %(name)s: %(message)s")
logger = logging.getLogger(__name__)

app = FastAPI(title="Health Mate API", version="1.0.0")

app.add_middleware(
    CORSMiddleware,
    allow_origins=settings.CORS_ORIGINS,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)


class ServiceContainer:
    def __init__(self) -> None:
        self._smtp = None
        self._gspread = None
        self._patient_db = None
        self._device_db = None
        self._doctor_db = None
        self._ml = None

    @property
    def smtp(self) -> SMTPserver:
        if self._smtp is None:
            self._smtp = SMTPserver(SMTP_USERNAME=settings.SMTP_USERNAME, SMTP_PASSWORD=settings.SMTP_PASSWORD)
        return self._smtp

    @property
    def gspread(self) -> GspreadConnection:
        if self._gspread is None:
            self._gspread = GspreadConnection(
                SCOPES=settings.SCOPES,
                SERVICE_ACCOUNT_FILE=settings.SERVICE_ACCOUNT_FILE,
                sheet_name=settings.GOOGLE_SHEET_NAME,
            )
        return self._gspread

    @property
    def patient_db(self) -> MongoConnection:
        if self._patient_db is None:
            self._patient_db = MongoConnection(
                username=settings.MONGO_USERNAME,
                password=settings.MONGO_PASSWORD,
                cluster_uri=settings.MONGO_CLUSTER_URI,
                databaseName=settings.MONGO_DB_NAME,
                collectionName="profile data",
            )
        return self._patient_db

    @property
    def device_db(self) -> MongoConnection:
        if self._device_db is None:
            self._device_db = MongoConnection(
                username=settings.MONGO_USERNAME,
                password=settings.MONGO_PASSWORD,
                cluster_uri=settings.MONGO_CLUSTER_URI,
                databaseName=settings.MONGO_DB_NAME,
                collectionName="device data",
            )
        return self._device_db

    @property
    def doctor_db(self) -> MongoConnection:
        if self._doctor_db is None:
            self._doctor_db = MongoConnection(
                username=settings.MONGO_USERNAME,
                password=settings.MONGO_PASSWORD,
                cluster_uri=settings.MONGO_CLUSTER_URI,
                databaseName=settings.MONGO_DB_NAME,
                collectionName="doctors data",
            )
        return self._doctor_db

    @property
    def ml(self) -> mlModel:
        if self._ml is None:
            self._ml = mlModel(xgmodel_path=settings.XG_MODEL_PATH, bpmodel_path=settings.BP_MODEL_PATH)
        return self._ml


services = ServiceContainer()


class User(BaseModel):
    name: str
    email: str
    password: str
    phNo: int
    age: int
    bloodGroup: str
    gender: str


class Doctor(BaseModel):
    registration: int
    name: str
    email: str
    password: str
    phNo: int
    age: int
    bloodGroup: str
    gender: str


class SensorData(BaseModel):
    heart_rate: float
    spo2: float
    temperature: float
    ECGSignal: float
    PPG: list[int] = []


class DeviceAlert(BaseModel):
    value: float
    id: str


@app.get("/")
async def root():
    return {"status": "online", "service": "Health Mate Backend API"}


@app.post("/assign/")
async def assign_device(deviceid: int, doctorid: int):
    try:
        device_doc = services.device_db.initDevice(deviceid=deviceid)
        clientid = device_doc.get("client", 0) if device_doc else 0
        res = services.doctor_db.assignDevice(deviceid=deviceid, doctorid=doctorid, clientid=clientid)
        return {"report": "positive", "response": res}
    except Exception as exc:
        logger.exception("Failed to assign device %s to doctor %s", deviceid, doctorid)
        return {"report": "negative", "error": str(exc)}


@app.post("/patient/")
async def assign_patient(doctorid: int, patientid: int, deviceid: int):
    try:
        services.device_db.assignConnection(deviceid=deviceid, clientid=patientid)
        res = services.doctor_db.assignUser(doctorid=doctorid, clientid=patientid, deviceid=deviceid)
        return {"report": "positive", "response": res}
    except Exception as exc:
        logger.exception("Failed to assign patient %s to device %s", patientid, deviceid)
        return {"report": "negative", "error": str(exc)}


@app.post("/append/")
async def append_sensor_data(_id: int, data: SensorData):
    try:
        payload = data.model_dump()
        clientid = services.device_db.getClientID(deviceid=_id)
        if not clientid:
            return {"report": "negative", "error": "No patient assigned to this device"}

        stress_level = services.ml.stressCalculation([payload["heart_rate"]])
        sbp, dbp = services.ml.bpCalculation(SignalInput=payload["PPG"])

        if payload["spo2"] <= 0:
            payload["spo2"] = -1
        if payload["heart_rate"] >= 200 or payload["heart_rate"] <= 40:
            payload["heart_rate"] = -1
            stress_level = -1
        if payload["temperature"] <= 0:
            payload["temperature"] = -1

        row = [
            datetime.now().strftime("%m/%d/%Y, %H:%M:%S"),
            payload["spo2"],
            payload["temperature"],
            payload["heart_rate"],
            payload["ECGSignal"],
            stress_level,
            sbp,
            dbp,
        ]
        res = services.gspread.appendData(_id=str(clientid), data=row)
        return {"report": "positive", "response": res}
    except Exception as exc:
        logger.exception("Error appending sensor data for device %s", _id)
        return {"report": "negative", "error": str(exc)}


@app.post("/adduser/")
async def add_user(user: User):
    data = user.model_dump()
    existing = services.patient_db.findUserId(email=data["email"])
    if existing:
        return None

    data["_id"] = random.randint(100000, 999999)
    try:
        services.smtp.sendID(gretingSystem=gretingSystem(value=data["_id"]), receiver_email=data["email"])
        services.patient_db.createData(data=data)
        services.gspread.addWorksheet(_id=str(data["_id"]))
        return {"report": "positive", "message": "operation successful", "id": data["_id"]}
    except Exception as exc:
        logger.exception("Error registering patient %s", data["email"])
        return {"report": "negative", "error": str(exc)}


@app.post("/adddoctor/")
async def add_doctor(user: Doctor):
    data = user.model_dump()
    existing = services.doctor_db.findUserId(email=data["email"])
    if existing:
        return None

    data["_id"] = random.randint(100000, 999999)
    data["devices"] = {}
    try:
        services.smtp.sendID(gretingSystem=gretingSystem(value=data["_id"]), receiver_email=data["email"])
        services.doctor_db.createData(data=data)
        return {"report": "positive", "message": "operation successful", "id": data["_id"]}
    except Exception as exc:
        logger.exception("Error registering doctor %s", data["email"])
        return {"report": "negative", "error": str(exc)}


@app.get("/show/")
async def show_patient_vitals(_id: str):
    try:
        return services.gspread.showData(_id=str(_id))
    except Exception as exc:
        return {"report": "negative", "error": str(exc)}


@app.get("/find/")
async def find_patient(_id: str):
    try:
        return services.patient_db.find(_id=int(_id))
    except Exception as exc:
        return {"report": "negative", "error": str(exc)}


@app.get("/getdoctor/")
async def get_doctor(_id: str):
    try:
        return services.doctor_db.find(_id=int(_id))
    except Exception as exc:
        return {"report": "negative", "error": str(exc)}


@app.get("/getdevices/")
async def get_doctor_devices(_id: str):
    try:
        res = services.doctor_db.find(_id=int(_id))
        return res.get("devices", {}) if res else {}
    except Exception as exc:
        return {"report": "negative", "error": str(exc)}


@app.get("/finduser/")
async def find_user_id(email: str, password: str):
    try:
        return services.patient_db.findUserId(email=email, password=password)
    except Exception as exc:
        return {"report": "negative", "error": str(exc)}


@app.get("/finddoctor/")
async def find_doctor_id(email: str, password: str):
    try:
        return services.doctor_db.findUserId(email=email, password=password)
    except Exception as exc:
        return {"report": "negative", "error": str(exc)}


@app.post("/sendmail/")
async def send_emergency_mail(device: DeviceAlert):
    payload = device.model_dump()
    device_id = int(payload["id"])
    if not device_id:
        return {"report": "negative", "error": "id not assigned"}

    try:
        client_id = services.device_db.getClientID(deviceid=device_id)
        if not client_id:
            return {"report": "negative", "error": "No patient mapped to device"}
        user_det = services.patient_db.find(_id=int(client_id))
        if not user_det or "email" not in user_det:
            raise HTTPException(status_code=404, detail="Patient email not found")
        return services.smtp.sendAlert(alertText=alertText(value=payload["value"]), receiver_email=user_det["email"])
    except Exception as exc:
        logger.exception("Failed sending emergency alert for device %s", device_id)
        return {"report": "negative", "error": str(exc)}


@app.get("/data/")
async def get_latest_spo2(_id: int):
    try:
        clientid = services.device_db.getClientID(deviceid=_id)
        return services.gspread.lastData(str(clientid))
    except Exception as exc:
        return {"report": "negative", "error": str(exc)}


@app.get("/test/")
async def get_server_time():
    return datetime.now().strftime("%m/%d/%Y, %H:%M:%S")
