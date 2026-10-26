import sys
import socket

sys.path.insert(0, 'sys/lib/omsy')
try:
	from omsycgi import CGI
except ImportError:
	sys.exit(1)

# DEBUG
from pathlib import Path
dossier_actuel = Path.cwd()
print(dossier_actuel)
# DEBUG

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

	__cgi = CGI(request, sock)
	body = __cgi.open(__cgi.templatepath(__cgi.path))
	if(body == None):
		body = "<html><body><h1>Welcome to Omsy Server Python / C</h1></body></html>"


	response = __cgi.response(body)
	sock.sendall(response)
	# DEBUG
	sock.close()

sock.close()
print("[Python] Fermeture.", file=sys.stderr)