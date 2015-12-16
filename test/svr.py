import BaseHTTPServer
class MyHandler(BaseHTTPServer.BaseHTTPRequestHandler):
  def do_GET(s):
    s.protocol_version='HTTP/1.1'
    s.send_response(200)
    if s.path in ('/echo', '/index'):
      s.send_header('Content-Type', 'application/octet-stream')
      s.send_header('Content-Length', str(len('echo')))
      s.end_headers()
      s.wfile.write(s.path[1:])

  def do_POST(s):
    s.protocol_version='HTTP/1.1'
    content_len = int(s.headers.getheader('content-length', -1))
    if content_len>0:
      data = s.rfile.read(content_len)
      print 'req content len: ', content_len
      #print 'recv data: ', data
      print 'req path: ', s.path
    else:
      while True:
        data = s.rfile.read(100)
        if data is None:
          break

    if s.path == '/echo':
      s.send_response(200)	
      s.send_header('Content-Type', 'application/octet-stream')
      s.send_header('Content-Length', str(len(data)))
      s.end_headers()
      s.wfile.write(data)
      
    elif s.path == '/upload':
      wfname = '/tmp/uploadfile.dat'
      try:	
        f = open(wfname, 'wb')
        f.write(data)
      except:
        pass      
      f.close()
      s.send_response(200)  
      s.send_header('Content-Type', 'application/octet-stream')
      s.send_header('Content-Length', str(len(wfname)))
      s.end_headers()
      s.wfile.write(wfname)
    elif s.path == '/quit':
      s.send_response(200)  
      httpd.server_close()

server_class = BaseHTTPServer.HTTPServer

httpd = server_class(('127.0.0.1', 9000), MyHandler)
try:
  httpd.serve_forever()
except:
  pass
httpd.server_close()
