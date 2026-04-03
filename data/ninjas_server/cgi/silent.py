#!/usr/bin/env python3
import os
import sys
import urllib.parse

STATE_FILE = os.path.join(os.path.dirname(__file__), "silent_state.txt")

# -------------- helper function --------------
def read_state():
    try:
        with open(STATE_FILE, "r") as f:
            return f.read().strip()
    except FileNotFoundError:
        return "OFF"

def write_state(state):
    with open(STATE_FILE, "w") as f:
        f.write(state)

# ------------------ read GET ------------------
query_string = os.environ.get("QUERY_STRING", "")
get_params = urllib.parse.parse_qs(query_string)

# ------------------ read POST ------------------
method = os.environ.get("REQUEST_METHOD", "")
post_params = {}

if method == "POST":
    length = int(os.environ.get("CONTENT_LENGTH", 0))
    body = sys.stdin.read(length)
    post_params = urllib.parse.parse_qs(body)

# ------------------ current statte ------------------
current_state = read_state()

# ------------------ POST logic ------------------
if method == "POST":
    if current_state == "ON":
        current_state = "OFF"
    else:
        current_state = "ON"

    write_state(current_state)

# ------------------ HTML ------------------
print("Content-Type: text/html")
print() 

html = f"""
<!DOCTYPE html>
<html>
<head>
    <title>Silent Mode</title>
    <link rel="stylesheet" href="/style.css">
</head>

<body class="bg-silent center-container">
    <p class="title">Silent Mode</p>
    <p class="subtitle">Current state: <b>{current_state}</b></p>

    <form method="POST" action="/silent/silent.py">
        <button class="btn" type="submit">Toggle Silent Mode</button>
    </form>
</body>
</html>
"""

print(html)
