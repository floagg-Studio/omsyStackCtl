import os

class CGI:
	webinterface = "usr/webinterface/"
	method = ""
	path = ""

	def __init__(self, request_bytes, sock):
		headers_part, _, body_part = request_bytes.partition(b"\r\n\r\n")
		lines = headers_part.split(b"\r\n")
		request_line = lines[0].decode(errors="replace")
		self.method, self.path, _ = request_line.split(" ")
		self.path = self.path[1:]
		if(self.path == ''):
			self.path = 'index.html'

		headers_dict = {}
		for line in lines[1:]:
			if b":" in line:
				key, val = line.split(b":", 1)
				key = key.decode().strip().lower()
				val = val.decode().strip()
				headers_dict[key] = val

		content_length = int(headers_dict.get('content-length', 0))
		body = body_part
		while len(body) < content_length:
			body += sock.recv(4096)


		print(self.templatepath(self.path))

	def templatepath(self, filename):
		return self.webinterface+filename

	def open(self, filename):
		if os.path.isfile(filename) != True:
			return None

		with open(filename, 'r') as f:
			contenu = f.read()

		return contenu

	def response(self, body = ""):
		response = (
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/html; charset=utf-8\r\n"
			f"Content-Length: {len(body.encode('utf-8'))}\r\n"
			"Connection: keep-alive\r\n"
			"\r\n"
			f"{body}"
		)

		return response.encode('utf-8')