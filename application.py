#!/usr/bin/env python3

"""
    Author: donsky
    For: www.donskytech.com
    Purpose: Create a REST Server Interface using Bottle for future IOT Projects
"""

from bottle import route, run, request, get, response, default_app
from paste import httpserver
import sqlite3
import json
from pathlib import Path

# NOTE: CHANGE THIS TO WHERE YOU DOWNLOAD YOUR GIT REPO
db_folder = Path("C:/Users/Magnificent/PycharmProjects/database-project-master/StudentDB.db")

application = default_app()


@get('/student/isauthorized')
def message():
    rf_id_code = request.query.rf_id_code.lstrip().rstrip()
    length = len(rf_id_code)
    print(f"Received the following query parameter rf_id_code={rf_id_code}, len={length}")

    conn = sqlite3.connect(db_folder)
    cursor = conn.cursor()

    cursor.execute("SELECT COUNT(*) FROM STUDENTS WHERE RF_ID_CODE=?", (rf_id_code,))
    result = cursor.fetchone()
    row_count = result[0]
    print(f"query result :: ", row_count);
    cursor.close()

    # Set Response Header to JSON
    response.headers['Content-Type'] = 'application/json'
    response.headers['Cache-Control'] = 'no-cache'

    if (row_count > 0):
        message_result = {"is_authorized": "true"}
    else:
        message_result = {"is_authorized": "false"}
    print(f"message_result :: {message_result}")
    return json.dumps(message_result)


httpserver.serve(application, host='0.0.0.0', port=8080)
