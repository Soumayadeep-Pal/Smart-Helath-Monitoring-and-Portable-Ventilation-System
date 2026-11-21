import os
from dotenv import load_dotenv

load_dotenv()


class Settings:
    MONGO_USERNAME: str = os.getenv("MONGO_USERNAME", "")
    MONGO_PASSWORD: str = os.getenv("MONGO_PASSWORD", "")
    MONGO_CLUSTER_URI: str = os.getenv("MONGO_CLUSTER_URI", "")
    MONGO_DB_NAME: str = os.getenv("MONGO_DB_NAME", "HealthCare")

    SMTP_USERNAME: str = os.getenv("SMTP_USERNAME", "")
    SMTP_PASSWORD: str = os.getenv("SMTP_PASSWORD", "")

    SERVICE_ACCOUNT_FILE: str = os.getenv("GOOGLE_SERVICE_ACCOUNT_FILE", "credentials/service_account.json")
    GOOGLE_SHEET_NAME: str = os.getenv("GOOGLE_SHEET_NAME", "sensor new data")
    SCOPES: list[str] = [
        "https://spreadsheets.google.com/feeds",
        "https://www.googleapis.com/auth/drive",
    ]

    XG_MODEL_PATH: str = os.getenv("XG_MODEL_PATH", "models/xgboost_stress.json")
    BP_MODEL_PATH: str = os.getenv("BP_MODEL_PATH", "best_model.pt")

    CORS_ORIGINS: list[str] = [
        origin.strip() for origin in os.getenv("CORS_ORIGINS", "*").split(",") if origin.strip()
    ]


settings = Settings()
