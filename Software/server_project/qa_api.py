from flask import Flask, jsonify, request

from gemini_service import generate_text
from report_generator import save_report

app = Flask(__name__)


@app.post("/qa")
def qa_endpoint():
    data = request.get_json(silent=True) or {}
    question = data.get("question", "")
    if not question:
        return jsonify({"error": "question required"}), 400
    answer = generate_text(question)
    return jsonify({"answer": answer})


@app.post("/report/<user_id>")
def report_endpoint(user_id: str):
    report = save_report(user_id)
    return jsonify({"report": report})


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)
