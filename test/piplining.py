import socket
sck = socket.socket()
sck.connect(('127.0.0.1', 7000))
reqstr = 'GET /file HTTP/1.1\r\n'+'\r\n'
sck.send(reqstr)
echostr = 'POST /echo HTTP/1.1\r\n' + 'Content-Length: 5\r\n' + '\r\n'+ '1234\n'
sck.send(echostr)
f=open("/tmp/upload.dat", "wb");
while True:
	rcv = sck.recv(1000)
	f.write(rcv)
	

