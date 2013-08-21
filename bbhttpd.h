
#ifndef BBHTTPD_H_
#define BBHTTPD_H_

#include <stddef.h>

typedef struct
{
	const char* ip;
	int port;
	size_t max_request_size;
} bbhttpd_config_t;

typedef enum
{
	BBHTTPD_REQUEST_METHOD_GET,
	BBHTTPD_REQUEST_METHOD_POST
} bbhttpd_request_method_t;

typedef struct
{
	bbhttpd_request_method_t method;
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

bbhttpd_t* bbhttpd_start(const bbhttpd_config_t* config);
void bbhttpd_stop(bbhttpd_t* bbhttpd);

bbhttpd_request_t* bbhttpd_get_request(bbhttpd_t* bbhttpd);
void bbhttpd_send_response(bbhttpd_t* bbhttpd, bbhttpd_request_t* request, bbhttpd_response_t* response);

#endif

