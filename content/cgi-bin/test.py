#!/usr/bin/env python3
body = "Hello World"
print(f"HTTP/1.1 200 OK\r\nContent-Length: {len(body)}\r\nContent-Type: text/html\r\n\r\n{body}", end="")