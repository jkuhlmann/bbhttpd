
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

void bbhttpd_send_response(bbhttpd_request_t* request, bbhttpd_response_t* response);
void bbhttpd_decline_response(bbhttpd_request_t* request);

#ifdef __cplusplus
}
#endif

#endif

