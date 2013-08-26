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

#ifndef BBHTTPD_H_
#define BBHTTPD_H_

#include <stddef.h>
#include <stdlib.h>

typedef struct
{
	const char* ip;
	int port;
	size_t max_request_size;
	int blocking_accept;
	void* (*bbhttpd_malloc)(size_t size);
	void (*bbhttpd_free)(void* ptr);
} bbhttpd_config_t;

#define BBHTTPD_CONFIG_INIT	{ "0.0.0.0", 8080, 2048, 1, malloc, free }

typedef enum
{
	BBHTTPD_REQUEST_METHOD_GET,
	BBHTTPD_REQUEST_METHOD_POST,
	BBHTTPD_REQUEST_METHOD_UNSUPPORTED
} bbhttpd_request_method_t;

typedef struct
{
	void* raw;
	size_t raw_length;
	int fd;
} bbhttpd_request_t;

typedef struct
{
	int status;
	const void* body;
	size_t body_length;
} bbhttpd_response_t;

struct bbhttpd_t_;
typedef struct bbhttpd_t_ bbhttpd_t;

#ifdef __cplusplus
extern "C" {
#endif

bbhttpd_t* bbhttpd_start(const bbhttpd_config_t* config);
void bbhttpd_stop(bbhttpd_t* bbhttpd);

bbhttpd_request_t* bbhttpd_get_request(bbhttpd_t* bbhttpd);
bbhttpd_request_method_t bbhttpd_request_get_method(const bbhttpd_request_t* request);
size_t bbhttpd_request_get_path(const bbhttpd_request_t* request, char* path, size_t max_len);

void bbhttpd_send_response(bbhttpd_t* bbhttpd, bbhttpd_request_t* request, bbhttpd_response_t* response);
void bbhttpd_decline_response(bbhttpd_t* bbhttpd, bbhttpd_request_t* request);

#ifdef __cplusplus
}
#endif

#endif

