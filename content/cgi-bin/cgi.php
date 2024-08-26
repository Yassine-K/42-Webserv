#!/usr/bin/env php
<?php
$data = explode("&", urldecode($_ENV['QUERY_STRING']));

$body = <<<EOD
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
            <h1>Methode: {$_ENV['REQUEST_METHOD']} </h1>
            <h2>
EOD;

$body .= str_replace("=", ": ", $data[2]);
$body .= "</h2><h2>";
$body .= str_replace("=", ": ", $data[3]);
$body .= "</h2><h2>";
$body .= str_replace("=", ": ", $data[4]);
$body .= "</h2></div></body></html>";

echo "HTTP/1.1 200 OK\r\nContent-Length: " . strlen($body) . "\r\nContent-Type: text/html\r\n\r\n" . $body;

?>