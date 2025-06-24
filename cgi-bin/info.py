#!/usr/bin/python3

import os
import datetime

print("Content-type: text/html\n")

current_time = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")

print("""
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <title>Información del Servidor</title>
    <style>
        body {
            font-family: 'Segoe UI', sans-serif;
            background-color: #f4f4f4;
            color: #333;
            padding: 30px;
        }
        h1 {
            color: #2c3e50;
        }
        .time {
            font-size: 1.5em;
            margin: 20px 0;
            color: #16a085;
        }
        .env {
            background: #ffffff;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 0 10px rgba(0,0,0,0.1);
            max-height: 400px;
            overflow-y: auto;
        }
        .env ul {
            list-style: none;
            padding: 0;
        }
        .env li {
            margin-bottom: 10px;
        }
        .env strong {
            color: #2980b9;
        }
        .footer {
            margin-top: 30px;
            font-style: italic;
            color: #888;
        }
    </style>
</head>
<body>
    <h1>🖥️ Información del Servidor CGI</h1>
""")

print(f"""    <div class="time">Hora actual: {current_time}</div>""")

print("""
    <div class="env">
        <h2>Variables de entorno:</h2>
        <ul>
""")

for param in sorted(os.environ.keys()):
    print(f"<li><strong>{param}</strong>: {os.environ[param]}</li>")

print("""
        </ul>
    </div>
    <div class="footer">Generado dinámicamente con Python CGI.</div>
</body>
</html>
""")
