
#!/usr/bin/env python3
import os
import sys
import re

DATA_FILE = os.path.join(os.path.dirname(__file__), "../cgi/cadets.txt")

# Read POST data from stdin
content_length = int(os.environ.get('CONTENT_LENGTH', 0))
post_data = sys.stdin.buffer.read(content_length) if content_length > 0 else b''

# Parse multipart/form-data
content_type = os.environ.get('CONTENT_TYPE', '')

if 'multipart/form-data' in content_type:
    # Extract boundary
    boundary = content_type.split('boundary=')[1].encode()
    parts = post_data.split(b'--' + boundary)
    
    name = "unknown"
    filename = None
    
    for part in parts:
        if b'name="name"' in part:
            name = part.split(b'\r\n\r\n')[1].split(b'\r\n')[0].decode('utf-8', errors='replace')
        elif b'name="photo"' in part and b'filename=' in part:
            filename = part.split(b'filename="')[1].split(b'"')[0].decode('utf-8', errors='replace')
    
    name = re.sub(r'[^\w\s-]', '', name).strip()[:50]
    if not name:
        print("Content-Type: text/html\n")
        print("<h1>Error: Invalid name</h1>")
        exit()
    
    if not filename:
        print("Content-Type: text/html\n")
        print("<h1>Error: No file uploaded</h1>")
        exit()
    
    filename = os.path.basename(filename)
    filename = re.sub(r'[^\w\s.-]', '', filename)
    
    try:
        with open(DATA_FILE, "a") as f:
            f.write(f"{name},{filename}\n")
    except (OSError, IOError):
        print("Content-Type: text/html")
        print()
        print("<h1>Error: Could not save data</h1>")
        sys.stdout.flush()
        exit()
    
    print("Status: 303 See Other")
    print("Location: /cadets_list")
    print("Content-Type: text/html")
    print()
    print("<html><body>Redirecting...</body></html>")
    sys.stdout.flush()
else:
    print("Content-Type: text/html")
    print()
    print("<h1>Error: Invalid content type</h1>")
    sys.stdout.flush()