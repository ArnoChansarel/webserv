#!/usr/bin/env python3

print("""
<!DOCTYPE html>
<html>
<head>
    <title>CGI Python Example</title>
    <style>
        p {
            color: blue;
            font-size: 16px;
        }
    </style>
</head>
<body>
    <h1>Hello, CGI Python!</h1>
    <p>This is an example of executing Python script via CGI.</p>
    <button onclick="location.href='infinite.py'">Go infinite</button>
    <button onclick="location.href='upload.py'">Upload a file</button>
    <button onclick="location.href='fast_bad_calculator.py'">SUPER KALKULATOR (beta phase)</button>
    <button onclick="location.href='/index.html'">Go back</button>
</body>
</html>
""")