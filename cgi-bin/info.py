#!/usr/bin/python3

import os
import sys
import datetime

# Send HTTP headers
print("Content-type: text/html\n")

# Start HTML output
print("""
<!DOCTYPE html>
<html>
<head>
    <title>CGI Test Info</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; }
        .info { background: #f0f0f0; padding: 20px; border-radius: 5px; }
        .time { color: #2c3e50; font-size: 2em; margin: 20px 0; }
    </style>
</head>
<body>
""")

# Current time
current_time = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
print(f"<div class='time'>Current Time: {current_time}</div>")

# Server information
print("<div class='info'>")
print("<h2>Environment Variables:</h2>")
print("<ul>")
for param in sorted(os.environ.keys()):
    print(f"<li><strong>{param}:</strong> {os.environ[param]}</li>")
print("</ul>")
print("</div>")

print("</body></html>")