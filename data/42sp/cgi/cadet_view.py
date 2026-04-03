#!/usr/bin/env python3
import os, urllib.parse, html

DATA_FILE = os.path.join(os.path.dirname(__file__), "../cadets/cadets.txt")
UPLOAD_DIR = "/cadets/uploads/"

# ------------------------------
# READ FILENAME
# ------------------------------
query = os.environ.get("QUERY_STRING", "")
params = urllib.parse.parse_qs(query)
filename = params.get("file", [""])[0]
# Sanitize filename to prevent path traversal
filename = os.path.basename(filename)

# ------------------------------
# SEARCH FOR CADET NAME
# ------------------------------
cadet_name = "Unknown"

try:
    with open(DATA_FILE, "r") as f:
        for line in f:
            name, photo = line.strip().split(",")
            if photo == filename:
                cadet_name = name
                break
except Exception:
    pass

# ------------------------------
# HTML RESPONSE
# ------------------------------
print("Content-Type: text/html")
print()

print(f"""
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Cadet {html.escape(cadet_name)}</title>
    <link rel="stylesheet" href="/style.css">
    <script>
        function doDelete() {{
            const name = "{html.escape(cadet_name)}";
            const file = "{urllib.parse.quote(filename)}";
            if (!confirm("Deletar " + name + "?")) return;
            fetch("/cgi/cadet_delete.py?name=" + name + "&file=" + file, {{
                method: 'DELETE'
            }}).then(() => window.location.href = "/cadets_list");
        }}
    </script>
</head>
<body class="bg">
    <p class="title">{html.escape(cadet_name)}</p>
    <img src="{UPLOAD_DIR}{html.escape(filename)}" style="max-width:400px; border-radius:12px; margin-top:20px">

    <div style="margin-top:20px">
        <button class="btn" onclick="doDelete()" style="background-color:red; color:white;">Delete Cadet</button>
    </div>
</body>
</html>
""")
