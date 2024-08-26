#!/usr/bin/env node
data = decodeURIComponent(process.env.QUERY_STRING).split('&')

// if (process.env.REQUEST_METHOD == 'POST')
// 	data = readLine()

body = `
	<!DOCTYPE html>
	<html lang="en">

	<head>
		<meta charset="UTF-8">
		<meta name="viewport" content="width=device-width, initial-scale=1.0">
		<title>CGI</title>
		<link rel="stylesheet" href="/cgi-bin/forms/styles/style.css" />
	</head>

	<body>
		<div class="response">
			<h1>Methode: ${process.env.REQUEST_METHOD}</h1>
			<h2>${data[2].replace('=', ': ')}</h2>
			<h2>${data[3].replace('=', ': ')}</h2>
			<h2>${data[4].replace('=', ': ')}</h2>
		</div>
	</body>

	</html>
`

console.log(`HTTP/1.1 200 OK\r\nContent-Length: ${body.length}\r\nContent-Type: text/html\r\n\r\n${body}`)