from __future__ import annotations

import json
from datetime import datetime, timedelta

from db.base import BaseService
from gemini_service import generate_text

CREDENTIAL_PATH = "env/sleep-system-7d563-firebase.json"
DATABASE_URL = "https://sleep-system-7d563-default-rtdb.asia-southeast1.firebasedatabase.app/"
DEFAULT_USER_ID = "2mrSt8vHRQd6kpPiHjuLobCrwK13"

service = BaseService(CREDENTIAL_PATH, DATABASE_URL)


def _fetch_last_two_days(user_id: str) -> tuple[dict[str, float], dict[str, float]]:
    """Return heart rate and SpO2 data from the last two days."""
    user = service.get_user(user_id)
    now = datetime.utcnow()
    start = now - timedelta(days=2)
    hr = {
        ts: rec.bpm
        for ts, rec in user.heart_rate.items()
        if datetime.fromisoformat(ts) >= start
    }
    spo2 = {
        ts: rec.percentage
        for ts, rec in user.spo2.items()
        if datetime.fromisoformat(ts) >= start
    }
    return hr, spo2


def generate_health_report(user_id: str = DEFAULT_USER_ID) -> str:
    """Generate a summary report and Q&A using Gemini."""
    hr, spo2 = _fetch_last_two_days(user_id)
    data_text = json.dumps({"heart_rate": hr, "spo2": spo2})
    prompt = (
        "You are a healthcare assistant. Using the following JSON data of heart rate "
        "and SpO2 measurements from the last two days, provide a short health "
        "summary followed by three helpful question and answer pairs:\n" + data_text
    )
    return generate_text(prompt)


def save_report(user_id: str = DEFAULT_USER_ID) -> str:
    """Generate and store the report in Firebase."""
    report = generate_health_report(user_id)
    service.set_health_report(user_id, report)
    return report


if __name__ == "__main__":
    print(save_report())
