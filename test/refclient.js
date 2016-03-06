var program = require('commander')

program
	.version('0.0.1')
	.option('-c --cmd [value]', 'Add Command')
	.parse(process.argv)

if(program.cmd) console.log('cmd: '+program.cmd);	

// var request = require('request');
// request('http://www.google.com', function (error, response, body) {
//   if (!error && response.statusCode == 200) {
//   console.log(body) // Show the HTML for the Google homepage. 
//   }
// })
