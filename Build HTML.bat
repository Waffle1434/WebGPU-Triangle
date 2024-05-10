CALL emcmake cmake -B build-web && cmake --build build-web -j4
echo Open: "http://127.0.0.1:8080/app.html" in Chrome
npx http-server build-web -S