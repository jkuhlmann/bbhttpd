
/* BBHTTPD test program
 *
 * Compile with:
 * gcc -Wall -Wextra -std=c89 -pedantic -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition -ansi -Werror -O -g *.c
 */

#include <stdio.h>
#include <string.h>
#include "bbhttpd.h"


int main(int argc, char* argv[])
{
	char path[512];

	bbhttpd_config_t config = BBHTTPD_CONFIG_INIT;
	bbhttpd_t* bbhttpd = bbhttpd_start(&config);

	(void)argc;
	(void)argv;

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
			bbhttpd_request_get_path(request, path, 512);
			printf("Got %d request on %s:\n%s", bbhttpd_request_get_method(request), path, (char*)request->raw);
			if (strcmp(path, "/testerror") == 0)
			{
				printf("Declining to respond\n");
				bbhttpd_decline_response(request);
			}
			else
			{
				const char* test_response = "HELLO BROWSER! HELLOHELLO!";
				bbhttpd_response_t response;
				response.status = 200;
				response.body = test_response;
				response.body_length = strlen(test_response);
				printf("Sending response\n");
				bbhttpd_send_response(request, &response);
			}
		}
	}

	bbhttpd_stop(bbhttpd);

	return 0;
}

