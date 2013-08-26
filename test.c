/* BBHTTPD - The barebones HTTP daemon
 *
 * Copyright (c) 2013 Johannes Kuhlmann
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

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
				bbhttpd_decline_response(bbhttpd, request);
			}
			else
			{
				const char* test_response = "HELLO BROWSER! HELLOHELLO!";
				bbhttpd_response_t response;
				response.status = 200;
				response.body = test_response;
				response.body_length = strlen(test_response);
				printf("Sending response\n");
				bbhttpd_send_response(bbhttpd, request, &response);
			}
		}
	}

	bbhttpd_stop(bbhttpd);

	return 0;
}

