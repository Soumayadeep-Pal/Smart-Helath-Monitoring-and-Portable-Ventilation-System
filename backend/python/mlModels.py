import os
import logging
import numpy as np
import torch
import torch.nn as nn
from xgboost import XGBRegressor

logger = logging.getLogger(__name__)

SBP_MAX = 180.0
DBP_MAX = 120.0


class CNNLSTM_BP(nn.Module):
    def __init__(self):
        super().__init__()
        self.conv1 = nn.Conv1d(1, 16, kernel_size=15, stride=2, padding=7)
        self.bn1 = nn.BatchNorm1d(16)
        self.dropout1 = nn.Dropout(0.3)
        self.conv2 = nn.Conv1d(16, 32, kernel_size=15, stride=2, padding=7)
        self.bn2 = nn.BatchNorm1d(32)
        self.dropout2 = nn.Dropout(0.3)
        self.conv3 = nn.Conv1d(32, 64, kernel_size=15, stride=2, padding=7)
        self.bn3 = nn.BatchNorm1d(64)
        self.dropout3 = nn.Dropout(0.3)
        self.lstm = nn.LSTM(input_size=64, hidden_size=64, batch_first=True)
        self.fc1 = nn.Linear(64, 64)
        self.fc2 = nn.Linear(64, 2)

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        x = x.unsqueeze(1)
        x = torch.relu(self.bn1(self.conv1(x)))
        x = self.dropout1(x)
        x = torch.relu(self.bn2(self.conv2(x)))
        x = self.dropout2(x)
        x = torch.relu(self.bn3(self.conv3(x)))
        x = self.dropout3(x)
        x = x.permute(0, 2, 1)
        _, (hn, _) = self.lstm(x)
        x = torch.relu(self.fc1(hn[-1]))
        return self.fc2(x)


class mlModel:
    def __init__(self, xgmodel_path: str, bpmodel_path: str) -> None:
        self.xgmodel = None
        if xgmodel_path and os.path.exists(xgmodel_path):
            self.xgmodel = XGBRegressor()
            self.xgmodel.load_model(xgmodel_path)
        else:
            logger.warning("XGBoost stress model not found at '%s'; fallback heuristic will be used.", xgmodel_path)

        self.device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
        self.bpModel = CNNLSTM_BP()
        if bpmodel_path and os.path.exists(bpmodel_path):
            state_dict = torch.load(bpmodel_path, map_location=self.device, weights_only=True)
            self.bpModel.load_state_dict(state_dict)
        else:
            logger.warning("BP model weights not found at '%s'; uninitialized weights loaded.", bpmodel_path)
        self.bpModel.to(self.device)
        self.bpModel.eval()

    def stressCalculation(self, data: list[float]) -> int:
        if self.xgmodel is not None:
            arr = np.array(data, dtype=np.float32).reshape(1, -1)
            return int(self.xgmodel.predict(arr)[0])
        hr = float(data[0]) if data else 75.0
        if hr < 60:
            return 1
        if hr <= 90:
            return 2
        if hr <= 115:
            return 3
        return 4

    @staticmethod
    def preprocess_signal(new_signal: np.ndarray, max_length: int = 2000) -> torch.Tensor:
        signal_mean = np.mean(new_signal)
        signal_std = np.std(new_signal) + 1e-6
        signal = (new_signal - signal_mean) / signal_std

        if len(signal) < max_length:
            signal = np.pad(signal, (0, max_length - len(signal)))
        else:
            signal = signal[:max_length]

        return torch.tensor(signal, dtype=torch.float32).unsqueeze(0)

    def bpCalculation(self, SignalInput: list[int | float]) -> tuple[float, float]:
        if not SignalInput:
            return 120.0, 80.0
        signal_np = np.asarray(SignalInput, dtype=np.float32)
        processed_signal = self.preprocess_signal(signal_np).to(self.device)

        with torch.no_grad():
            output = self.bpModel(processed_signal)
            sbp_pred, dbp_pred = output.squeeze().cpu().numpy()
            predicted_sbp = float(sbp_pred) * SBP_MAX
            predicted_dbp = float(dbp_pred) * DBP_MAX

        return round(predicted_sbp, 2), round(predicted_dbp, 2)
