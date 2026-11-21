import logging
import smtplib
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText

logger = logging.getLogger(__name__)


class SMTPserver:
    def __init__(self, SMTP_USERNAME: str, SMTP_PASSWORD: str, smtp_server: str = "smtp.gmail.com", smtp_port: int = 587) -> None:
        self.SMTP_USERNAME = SMTP_USERNAME
        self.SMTP_PASSWORD = SMTP_PASSWORD
        self.smtp_server = smtp_server
        self.smtp_port = smtp_port

    def _send_html_email(self, subject: str, html: str, receiver_email: str) -> bool:
        if not self.SMTP_USERNAME or not self.SMTP_PASSWORD:
            raise ValueError("SMTP credentials missing: configure SMTP_USERNAME and SMTP_PASSWORD.")

        msg = MIMEMultipart()
        msg["From"] = self.SMTP_USERNAME
        msg["To"] = receiver_email
        msg["Subject"] = subject
        msg.attach(MIMEText(html, "html"))

        with smtplib.SMTP(self.smtp_server, self.smtp_port, timeout=15) as server:
            server.starttls()
            server.login(self.SMTP_USERNAME, self.SMTP_PASSWORD)
            server.sendmail(self.SMTP_USERNAME, receiver_email, msg.as_string())
        logger.info("Email sent successfully to %s", receiver_email)
        return True

    def sendAlert(self, alertText: str, receiver_email: str) -> dict:
        subject = "Urgent medical alert; Immediate action required for patient emergency"
        self._send_html_email(subject=subject, html=alertText, receiver_email=receiver_email)
        return {"report": "positive", "message": "Alert email sent"}

    def sendID(self, gretingSystem: str, receiver_email: str) -> dict:
        subject = "You have successfully registered for Health Mate"
        self._send_html_email(subject=subject, html=gretingSystem, receiver_email=receiver_email)
        return {"report": "positive", "message": "Registration email sent"}
