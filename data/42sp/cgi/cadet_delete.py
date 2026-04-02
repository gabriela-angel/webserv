#!/usr/bin/env python3
import os, urllib.parse, re, sys

DATA_FILE = os.path.join(os.path.dirname(__file__), "./cadets.txt")
UPLOAD_DIR = os.path.join(os.path.dirname(__file__), "../cadets/uploads/")

# READ PARAMETER FILE
query = os.environ.get("QUERY_STRING", "")
params = urllib.parse.parse_qs(query)
name = params.get("name", [""])[0]
filename_raw = params.get("file", [""])[0]

filename = os.path.basename(filename_raw)
filename = re.sub(r'[^\w\s.-]', '', filename)

if not filename or not name:
    print("Status: 303 See Other\r")
    print("Location: /errors/400.html\r")
    print("\r")
    sys.exit()

target = os.path.join(UPLOAD_DIR, filename)
real_target = os.path.realpath(target)
real_upload_dir = os.path.realpath(UPLOAD_DIR)

if not real_target.startswith(real_upload_dir):
    print("Status: 303 See Other\r")
    print("Location: /errors/403.html\r")
    print("\r")
    sys.exit()

# DELETE FOTO
if os.path.exists(target):
    os.remove(target)

# REMOVE ENTRY FROM cadets.txt
new_lines = []
try:
    with open(DATA_FILE, "r") as f:
        for line in f:
            parts = line.strip().split(",")
            if len(parts) != 2 or name != parts[0] or filename != parts[1]:
                new_lines.append(line)
    with open(DATA_FILE, "w") as f:
        f.writelines(new_lines)
except IOError:
    print("Status: 500 Internal Server Error\r")
    print("Content-Type: text/html\r")
    print("\r")
    print("<h1>Error processing request</h1>")
    exit()

# REDIRECT FOR LIST
print("Status: 303 See Other\r")
print("Location: /cadets_list\r")
print("Content-Type: text/html\r")
print("\r")
print("<html><body>Redirecting...</body></html>")