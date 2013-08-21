
#include <stdio.h>
#include <string.h>
#include "bbhttpd.h"


int main(int argc, char* argv[])
{
	bbhttpd_config_t config;
	config.ip = "0.0.0.0";
	config.port = 8080;
	config.max_request_size = 2048;

	bbhttpd_t* bbhttpd = bbhttpd_start(&config);
	if (!bbhttpd)
	{
		printf("Unable to listen\n");
		return -1;
	}

	while (1)
	{
		bbhttpd_request_t* request = bbhttpd_get_request(bbhttpd);
		if (request)
		{
			((char*)request->raw)[request->raw_length] = 0;
			printf("Got request:\n%s", (char*)request->raw);
			printf("Sending response\n");
			bbhttpd_response_t response;
			response.status = 200;
			const char* test_response = "HELLO BROWSER! HELLOHELLO!";
			response.body = test_response;
			response.body_length = strlen(test_response);
			bbhttpd_send_response(bbhttpd, request, &response);
		}
	}

	bbhttpd_stop(bbhttpd);

	return 0;
}

