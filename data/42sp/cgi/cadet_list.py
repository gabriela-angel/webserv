#!/usr/bin/env python3
import os, html
import urllib.parse
import sys

print("Content-Type: text/html\r\n\r\n", end="")

DATA_FILE = os.path.join(os.path.dirname(__file__), "cadets.txt")

cadets_html = ""
page_title = "Cadet's List"

try:
    if os.path.exists(DATA_FILE):
        with open(DATA_FILE, "r") as f:
            for line in f:
                clean_line = line.strip()
                if not clean_line or "," not in clean_line:
                    continue
                
                parts = clean_line.split(",")
                if len(parts) == 2:
                    name, photo = parts
                    name_esc = html.escape(name)
                    photo_encoded = urllib.parse.quote(photo)
                    
                    cadets_html += f"""
                    <div class="card">
                        <img src="/cadets/uploads/{photo_encoded}" alt="{name_esc}">
                        <div class="card-content">
                            <div class="subtitle">{name_esc}</div>
                            <button type="button" class="delete-btn" onclick="doDelete('{name_esc}', '{photo_encoded}')">DELETE</button>
                        </div>
                    </div>
                    """
except Exception as e:
    sys.stderr.write(f"Erro na leitura: {str(e)}\n")

print(f"""<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>{page_title}</title>
    <link rel="stylesheet" href="/style.css">
    <link href="https://fonts.googleapis.com/css2?family=Science+Gothic&display=swap" rel="stylesheet">
    <script>
        function doDelete(name, file) {{
            if (!confirm("Deletar " + name + "?")) return;
            fetch(`./cgi/cadet_delete.py?name=${{encodeURIComponent(name)}}&file=${{encodeURIComponent(file)}}`, {{
                method: 'DELETE'
            }}).then(() => window.location.reload());
        }}
    </script>
</head>
<body class="bg">""")

if cadets_html:
    print(f'<div class="columns">{cadets_html}</div>')
else:
    print(f"""
    <div class="container">
        <h1 class="title">No cadets registered.</h1>
    </div>
    """)

print("</body></html>")