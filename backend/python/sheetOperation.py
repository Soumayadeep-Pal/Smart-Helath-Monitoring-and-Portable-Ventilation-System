import os
from datetime import datetime
import gspread
import pandas as pd
from oauth2client.service_account import ServiceAccountCredentials

COLUMNS = [
    "Date Time",
    "Blood Oxygen",
    "Temperature",
    "Heart Rate",
    "ECG Signal",
    "Stress Level",
    "SBP",
    "DBP",
]


class GspreadConnection:
    def __init__(self, SCOPES: list[str], SERVICE_ACCOUNT_FILE: str, sheet_name: str = "sensor new data") -> None:
        if not os.path.exists(SERVICE_ACCOUNT_FILE):
            raise FileNotFoundError(f"Google service account file not found: {SERVICE_ACCOUNT_FILE}")
        creds = ServiceAccountCredentials.from_json_keyfile_name(SERVICE_ACCOUNT_FILE, SCOPES)
        client = gspread.authorize(creds)
        self.sheet = client.open(sheet_name)

    def addWorksheet(self, _id: str):
        worksheet = self.sheet.add_worksheet(title=_id, rows=10000, cols=len(COLUMNS))
        worksheet.append_row(COLUMNS)
        return worksheet

    def appendData(self, _id: str, data: list):
        sheet_instance = self.sheet.worksheet(_id)
        return sheet_instance.append_row(data)

    def showData(self, _id: str) -> list[dict]:
        sheet_instance = self.sheet.worksheet(_id)
        raw_values = sheet_instance.get_all_values()
        if len(raw_values) <= 1:
            return []

        data = pd.DataFrame(raw_values[1:], columns=COLUMNS)
        data = data.replace("", "0").fillna(0)
        numeric_cols = {
            "Blood Oxygen": float,
            "Temperature": float,
            "Heart Rate": float,
            "ECG Signal": float,
            "Stress Level": float,
            "SBP": float,
            "DBP": float,
        }
        data = data.astype(numeric_cols).tail(30).reset_index(drop=True)
        return data.to_dict(orient="records")

    def lastData(self, _id: str) -> int:
        sheet_instance = self.sheet.worksheet(_id)
        oxygen_values = sheet_instance.col_values(2)
        if len(oxygen_values) > 1:
            timestamps = sheet_instance.col_values(1)
            latest_ts = timestamps[-1]
            elapsed = (datetime.now() - datetime.strptime(latest_ts, "%m/%d/%Y, %H:%M:%S")).total_seconds()
            if int(elapsed) <= 60:
                return int(float(oxygen_values[-1]))
        return 0
