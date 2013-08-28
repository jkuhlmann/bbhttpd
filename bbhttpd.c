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

#include "bbhttpd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

struct bbhttpd_t_
{
	int fd;
	size_t max_request_size;
	int single_request;
	bbhttpd_request_t* request;
	void* (*bbhttpd_malloc)(size_t size);
	void (*bbhttpd_free)(void* ptr);
};

static void bbhttpd_destroy_request(bbhttpd_t* bbhttpd, bbhttpd_request_t* request);

bbhttpd_t* bbhttpd_start(const bbhttpd_config_t* config)
{
	bbhttpd_t* bbhttpd = (bbhttpd_t*)config->bbhttpd_malloc(sizeof(bbhttpd_t));
	int sock_reuse_optval = 1;
	struct sockaddr_in addr;
	struct in_addr inp;

	bbhttpd->fd = socket(AF_INET, SOCK_STREAM, 0);
	if (bbhttpd->fd == -1)
	{
		config->bbhttpd_free(bbhttpd);
		return NULL;
	}

	if (!config->blocking_accept && fcntl(bbhttpd->fd, F_SETFL, O_NONBLOCK) == -1)
	{
		config->bbhttpd_free(bbhttpd);
		return NULL;
	}

	setsockopt(bbhttpd->fd, SOL_SOCKET, SO_REUSEADDR, &sock_reuse_optval, sizeof(sock_reuse_optval));

	if (!inet_pton(AF_INET, config->ip, &inp))
	{
		config->bbhttpd_free(bbhttpd);
		return NULL;
	}
	addr.sin_addr.s_addr = inp.s_addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(config->port);

	if (bind(bbhttpd->fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) == -1)
	{
		config->bbhttpd_free(bbhttpd);
		return NULL;
	}

	if (listen(bbhttpd->fd, 8) == -1)
	{
		config->bbhttpd_free(bbhttpd);
		return NULL;
	}

	bbhttpd->max_request_size = config->max_request_size;
	bbhttpd->bbhttpd_malloc = config->bbhttpd_malloc;
	bbhttpd->bbhttpd_free = config->bbhttpd_free;

	bbhttpd->single_request = config->single_request;
	if (bbhttpd->single_request)
	{
		bbhttpd->request =  (bbhttpd_request_t*)bbhttpd->bbhttpd_malloc(sizeof(bbhttpd_request_t));
		bbhttpd->request->raw = bbhttpd->bbhttpd_malloc(bbhttpd->max_request_size);
	}


	return bbhttpd;
}

void bbhttpd_stop(bbhttpd_t* bbhttpd)
{
	if (!bbhttpd)
	{
		return;
	}

	if (bbhttpd->single_request)
	{
		bbhttpd->bbhttpd_free(bbhttpd->request->raw);
		bbhttpd->bbhttpd_free(bbhttpd->request);
	}

	close(bbhttpd->fd);
	bbhttpd->bbhttpd_free(bbhttpd);
}

bbhttpd_request_t* bbhttpd_get_request(bbhttpd_t* bbhttpd)
{
	bbhttpd_request_t* request;
	struct sockaddr_in addr;
	socklen_t addr_len = sizeof(struct sockaddr_in);
	int recv_length;

	int remote_fd = accept(bbhttpd->fd, (struct sockaddr*)&addr, &addr_len);
	if (remote_fd == -1)
	{
		return NULL;
	}

	if (bbhttpd->single_request)
	{
		request = bbhttpd->request;
	}
	else
	{
		request = (bbhttpd_request_t*)bbhttpd->bbhttpd_malloc(sizeof(bbhttpd_request_t));
		request->raw = bbhttpd->bbhttpd_malloc(bbhttpd->max_request_size);
	}

	request->fd = remote_fd;

	recv_length = recv(remote_fd, request->raw, bbhttpd->max_request_size, 0);
	if (recv_length == -1)
	{
		bbhttpd->bbhttpd_free(request->raw);
		bbhttpd->bbhttpd_free(request);
		return NULL;
	}

	request->raw_length = recv_length;

	return request;
}

bbhttpd_request_method_t bbhttpd_request_get_method(const bbhttpd_request_t* request)
{
	if (strncmp("GET", (const char*)request->raw, 3) == 0)
		return BBHTTPD_REQUEST_METHOD_GET;
	else if (strncmp("POST", (const char*)request->raw, 4) == 0)
		return BBHTTPD_REQUEST_METHOD_POST;
	return BBHTTPD_REQUEST_METHOD_UNSUPPORTED;
}

size_t bbhttpd_request_get_path(const bbhttpd_request_t* request, char* path, size_t max_len)
{
	const char* text = (const char*)request->raw;
	const char* path_start = 0;
	const char* path_end = 0;
	size_t path_len;
	size_t copy_len;

	if (!path)
		return 0;

	for (path_start = text+1; path_start < text + request->raw_length && *(path_start-1) != ' '; ++path_start)
		;
	for (path_end = path_start; path_end < text + request->raw_length && *path_end != ' '; ++path_end)
		;

	if (!path_start || !path_end)
		return 0;

	path_len = path_end - path_start;
	copy_len = (path_len+1) < max_len ? path_len : max_len-1;
	strncpy(path, path_start, copy_len);
	path[copy_len] = 0;
	return copy_len;
}

static void bbhttpd_destroy_request(bbhttpd_t* bbhttpd, bbhttpd_request_t* request)
{
	close(request->fd);
	if (!bbhttpd->single_request)
	{
		bbhttpd->bbhttpd_free(request->raw);
		bbhttpd->bbhttpd_free(request);
	}
}

void bbhttpd_send_response(bbhttpd_t* bbhttpd, bbhttpd_request_t* request, bbhttpd_response_t* response)
{
	char buf[32];
	sprintf(buf, "HTTP/1.0 %d\r\n\r\n", response->status);
	send(request->fd, buf, strlen(buf), 0);
	send(request->fd, response->body, response->body_length, 0);
	bbhttpd_destroy_request(bbhttpd, request);
}

void bbhttpd_decline_response(bbhttpd_t* bbhttpd, bbhttpd_request_t* request)
{
	char buf[32];
	sprintf(buf, "HTTP/1.0 400\r\n\r\n");
	send(request->fd, buf, strlen(buf), 0);
	bbhttpd_destroy_request(bbhttpd, request);
}

