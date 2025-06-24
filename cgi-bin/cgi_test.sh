#!/bin/bash
echo "Content-Type: text/html"
echo ""

echo "<html>"
echo "<head><title>CGI Bash Test</title></head>"
echo "<body>"
echo "<h1>Hola desde CGI Bash!</h1>"
echo "<p>Hora actual del servidor: $(date)</p>"
echo "</body>"
echo "</html>"
