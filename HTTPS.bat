"C:\Program Files\Git\usr\bin\openssl.exe" req -newkey rsa:2048 -new -nodes -x509 -days 3650 -keyout key.pem -out cert.pem

npx http-server build-web -S