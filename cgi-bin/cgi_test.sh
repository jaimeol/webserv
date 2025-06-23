#!/bin/bash

# Esto es necesario para que el navegador lo interprete como HTML
echo "Content-Type: text/html"
echo ""

# Contenido HTML que devuelve el CGI
echo "<html>"
echo "<head><title>CGI Bash Test</title></head>"
echo "<body>"
echo "<h1>Hola desde CGI Bash!</h1>"
echo "<p>Hora actual del servidor: $(date)</p>"
echo "<p>Tu IP es: $REMOTE_ADDR</p>"
echo "</body>"
echo "</html>"
