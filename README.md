bbhttpd - The barebones HTTP daemon
===================================

A HTTP server that can easily be dropped into your existing project to help with debugging and development.

Features
--------
- Only one source file and one header file. Just drop the files into your project, no need to build a library and link to it.
- No unnecessary features. You get the HTTP request and send back a response.
- C89-compliant C code
- MIT license

Usage
-----

```c
#include "bbhttpd.h"

bbhttpd_config_t config = BBHTTPD_CONFIG_INIT;
bbhttpd_t* bbhttpd = bbhttpd_start(&config);

while (1)
{
	bbhttpd_request_t* request = bbhttpd_get_request(bbhttpd);
	if (request)
	{
		const char* test_response = "Hello, world!";
		bbhttpd_response_t response;
		response.status = 200;
		response.body = test_response;
		response.body_length = strlen(test_response);
		bbhttpd_send_response(request, &response);
	}
}

bbhttpd_stop(bbhttpd);
```

