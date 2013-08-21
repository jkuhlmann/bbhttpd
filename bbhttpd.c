
#include "bbhttpd.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>

struct bbhttpd_t_
{
	int fd;
	size_t max_request_size;
};

bbhttpd_t* bbhttpd_start(const bbhttpd_config_t* config)
{
	bbhttpd_t* bbhttpd = malloc(sizeof(bbhttpd_t));

	bbhttpd->fd = socket(AF_INET, SOCK_STREAM, 0);
	if (bbhttpd->fd == -1)
	{
		free(bbhttpd);
		return NULL;
	}

	if (fcntl(bbhttpd->fd, F_SETFL, O_NONBLOCK) == -1)
	{
		free(bbhttpd);
		return NULL;
	}

	int optval = 1;
	setsockopt(bbhttpd->fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	struct sockaddr_in addr;
	struct in_addr inp;
	if (!inet_aton(config->ip, &inp))
	{
		free(bbhttpd);
		return NULL;
	}
	addr.sin_addr.s_addr = inp.s_addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(config->port);

	if (bind(bbhttpd->fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) == -1)
	{
		free(bbhttpd);
		return NULL;
	}

	if (listen(bbhttpd->fd, 8) == -1)
	{
		free(bbhttpd);
		return NULL;
	}

	bbhttpd->max_request_size = config->max_request_size;

	return bbhttpd;
}

void bbhttpd_stop(bbhttpd_t* bbhttpd)
{
	if (!bbhttpd)
	{
		return;
	}
	close(bbhttpd->fd);
	free(bbhttpd);
}

bbhttpd_request_t* bbhttpd_get_request(bbhttpd_t* bbhttpd)
{
	struct sockaddr_in addr;
	socklen_t addr_len = sizeof(struct sockaddr_in);

	int remote_fd = accept(bbhttpd->fd, (struct sockaddr*)&addr, &addr_len);
	if (remote_fd == -1)
	{
		return NULL;
	}

	bbhttpd_request_t* request = malloc(sizeof(bbhttpd_request_t));
	request->raw = malloc(bbhttpd->max_request_size);
	request->fd = remote_fd;

	int length = recv(remote_fd, request->raw, bbhttpd->max_request_size, 0);
	if (length == -1)
	{
		free(request->raw);
		free(request);
		return NULL;
	}

	request->raw_length = length;

	return request;
}

void bbhttpd_send_response(bbhttpd_t* bbhttpd, bbhttpd_request_t* request, bbhttpd_response_t* response)
{
	char buf[32];
	snprintf(buf, 32, "HTTP/1.0 %d\r\n\r\n", response->status);
	send(request->fd, buf, strlen(buf), 0);
	send(request->fd, response->body, response->body_length, 0);
	close(request->fd);
	free(request->raw);
	free(request);
}

