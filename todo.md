# src
https://linux.die.net/man/3/libuuid
https://developer.mozilla.org/en-US/docs/Web/HTTP/Cookies
https://developer.mozilla.org/en-US/docs/Web/HTTP/Authentication

# map

!uuid || signin -> login [continue=/, user, password] <> -> generate uuid, set cookie -> continue
                                                         -> 401

failed parsing

GET /login HTTP/1.1
Host: localhost
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/125.0.0.0 Safari/537.36 GLS/100.10.9939.100
Accept: */*
Accept-Language: en-US,en;q=0.5
Accept-Encoding: gzip, deflate, br, zstd
Referer: https://localhost/login
Authorization: Basic Og==
DNT: 1
Sec-GPC: 1
Connection: keep-alive
Sec-Fetch-Dest: empty
Sec-Fetch-Mode: cors
Sec-Fetch-Site: same-origin
sec-ch-ua-platform: "Windows"
sec-ch-ua: "Google Chrome";v="125", "Chromium";v="125", "Not=A?Brand";v="24"
sec-ch-ua-mobile: ?0
Priority: u=0

change parsing only on 
