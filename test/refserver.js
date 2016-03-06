/**
 * New node file
 */

var express = require('express');
var app = express();

//
var bodyParser = require('body-parser'); // refer: https://www.npmjs.com/package/body-parser
var rawoctet = bodyParser.raw({ type: 'application/octet-stream', limit:100*1024*1024 });
var textplain = bodyParser.raw({ type: 'text/plain' });

// app.use(express.static('public'));
/* app.use(function (req, res, next) {
	  console.log('Time:', Date.now());
	  next();
	});
	
app.use('/user/:id', function (req, res, next) {
	  console.log('Request Type:', req.method);
	  next();
	});
*/
//app.get('/', function(req, res) {
//	res.send('Hello, express');
//});

app.get('/', function (req, res) {
	  res.render('index', { title: 'Hey', message: 'Hello there!'});
	});


app.get('/hello', function(req, res) {
	res.status(200).send();
});

app.post('/upload', rawoctet, function(req, res) {
	console.log('post upload,...');
//	console.log(req.body.toString());
	var fs = require('fs');
	var filepath = '/tmp/upload.dat';
//	
	fs.writeFile(filepath, req.body);
	res.send("OK");
});


app.post('/echo', rawoctet, function(req, res) {
	console.log('post upload,...');
	res.send(req.body);
});

app.get('/exit', function(req, res) {
	res.status(200).send()
	process.exit();
});

app.get('/delay', function(req, res) {
	setTimeout(function() {
		res.send("delayed response\n");
	}, 1000);
//	res.sendFile('/home/netmind/Pictures/nodejs.png');
});

var server = app.listen(7000, function() {
	var host = server.address().address;
	var port = server.address().port;

	console.log("<<<<< refserver start listening at http://%s:%s >>>>", host, port);	
});