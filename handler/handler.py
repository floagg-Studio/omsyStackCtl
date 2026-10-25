import sys
import socket

fd = int(sys.argv[1])
sock = socket.socket(fileno=fd)
print("[Python] Welcome.", file=sys.stderr)
while True:
	request = b""
	try:
		while b"\r\n\r\n" not in request:
			chunk = sock.recv(4096)
			if not chunk:
				break
			request += chunk
	except (socket.timeout, BlockingIOError, OSError) as e:
		print(f"[Python] TIMEOUT ({e})", file=sys.stderr)
		break
	if not request:
		print("[Python] CONNEXION FERMEE par le client", file=sys.stderr)
		break

	body = "<html><body><h1>Welcome to Omsy Server Python / C</h1></body></html>"
	with open("index.html", 'r') as f:
		body = f.read()

	response = (
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/html; charset=utf-8\r\n"
		f"Content-Length: {len(body.encode('utf-8'))}\r\n"
		"Connection: keep-alive\r\n"
		"\r\n"
		f"{body}"
	)
	sock.sendall(response.encode("utf-8"))
	print('[Python] Reponse sended', file=sys.stderr)

sock.close()
print("[Python] Fermeture.", file=sys.stderr)