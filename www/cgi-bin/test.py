#!/usr/bin/python3
import os

print("Content-Type: text/html\r\n\r\n")
print("<html><body>")
print("<h1>Hi!</h1>")
print("<ul>")
for param in os.environ.keys():
    print(f"<li>{param} = {os.environ[param]}</li>")
print("</ul>")
print("</body></html>")