#!/usr/bin/env python
import requests
import socket
import time
import threading
import thread
import platform

BASE_URL='http://127.0.0.1:6060'

def multi_auto_get():
	try:
		thread.start_new_thread(auto_get, ("th1", "auto-send"))
		thread.start_new_thread(auto_get, ("th2", "auto-send"))
		thread.start_new_thread(auto_get, ("th2", "auto-send"))
		thread.start_new_thread(auto_get, ("th3", "auto-send"))
		thread.start_new_thread(manual_long, ('1', 'test'))
		thread.start_new_thread(auto_get, ("th2", "auto-send"))
		thread.start_new_thread(auto_get, ("th3", "auto-send"))
	except:
		print '### Error'

def multi_auto_file():
	thread.start_new_thread(auto_file,('text',))
	thread.start_new_thread(auto_file, ('bin',))
	pass

def auto_get(thname, arg):
	r = requests.get('http://127.0.0.1:6060/'+arg)
	assert(r.text=='auto-send')
	print 'auto-send, recv: ', r.text

def manual_long(thname, arg):
	r = requests.get('http://127.0.0.1:6060/manual-long-send')
	print 'manual-long, recv len: ', len(r.text)


#hs='GET /manual-long-send HTTP/1.1\r\n'
#hs += 'Content-Length: 0\r\n'
#hs += '\r\n'
#s = socket.socket()
#s.connect(('127.0.0.1',  6060))
#s.send(hs)
#totalrcnt = 0

#while True:
	#r=s.recv(1024*1024)
	#if len(r)>0:
		#totalrcnt += len(r)
		#print 'total rcnt: ', totalrcnt
	#time.sleep(1)
	
#r = requests.get('http://127.0.0.1:6060/manual-long-send')
#print r.text

#r = requests.get('http://127.0.0.1:6060/manual-periodic-send')
#print 'data len: ', len(r.text)

def auto_file(ctype):
	r = requests.get(BASE_URL+'/auto-file')
	if ctype=='text':
		print 'text file...'
		r.text
	else:
		print 'bin file...'
	print 'recv len:', len(r.content)
	f = open('/boot/initrd.img-'+platform.uname()[2])
	d = f.read()
	if d != r.content:
		print '### Error: auto-file'
		exit(1)
	#f.close()
	print 'file close'



def tcp_download(inst_name):
	s = socket.socket()
	s.connect(('127.0.0.1', 6060))
	hs='GET /auto-file HTTP/1.1\r\n\r\n'
	s.send(hs)
	r=[]
	s.settimeout(5.0)
	while True:
		try:
			r+=s.recv(1*1024*1024)
			time.sleep(1000)
			print 'inst: %s, len: %d' %(inst_name, len(r))
		except:
			break;
	print inst_name, ' :', len(r)

		
def delay_auto_get():
	thread.start_new_thread(tcp_download, ('1',))
	thread.start_new_thread(tcp_download, ('2',))
	while True:
		time.sleep(1)

#multi_auto_file()		

delay_auto_get()
