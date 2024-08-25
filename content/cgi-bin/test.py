body = """
	salam
"""
print("HTTP/1.1 200 OK", end="\r\n")
print(f"Content-Lenght: {len(body)}", end="\r\n")
print('\r')
print(body)
