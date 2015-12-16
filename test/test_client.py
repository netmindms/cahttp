import requests

r = requests.get('http://127.0.0.1:6060/auto-send')
if r.text != 'auto-send':
	exit(1)
r = requests.get('http://127.0.0.1:6060/manual-te')
print r.status_code
print r.text

