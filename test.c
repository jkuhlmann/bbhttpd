
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
			char path[512];
			bbhttpd_request_get_path(request, path, 512);
			printf("Got %d request on %s:\n%s", bbhttpd_request_get_method(request), path, (char*)request->raw);
			if (strcmp(path, "/testerror") == 0)
			{
				printf("Declining to respond\n");
				bbhttpd_decline_response(request);
			}
			else
			{
				printf("Sending response\n");
				bbhttpd_response_t response;
				response.status = 200;
				const char* test_response = "HELLO BROWSER! HELLOHELLO!";
				response.body = test_response;
				response.body_length = strlen(test_response);
				bbhttpd_send_response(request, &response);
			}
		}
	}

	bbhttpd_stop(bbhttpd);

	return 0;
}

