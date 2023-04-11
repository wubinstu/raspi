//
// Created by Einc on 2023/03/21.
//

#ifndef __WEB_HTTP_H_
#define __WEB_HTTP_H_

#include "types.h"
#include "global.h"
#include "arr.h"

#define HTTP_REQ_GET        "GET"
#define HTTP_REQ_POST       "POST"
#define HTTP_REQ_HEAD       "HEAD"
#define HTTP_REQ_PUT        "PUT"
#define HTTP_REQ_DELETE     "DELETE"
#define HTTP_REQ_OPTIONS    "OPTIONS"
#define HTTP_REQ_TRACE      "TRACE"
#define HTTP_REQ_CONNECT    "CONNECT"

#define HTTP_VER_09         "HTTP/0.9"
#define HTTP_VER_10         "HTTP/1.0"
#define HTTP_VER_11         "HTTP/1.1"
#define HTTP_VER_20         "HTTP/2.0"


#define HTTP_REQ_HEAD_HOST                  "Host: "
#define HTTP_REQ_HEAD_REFERER               "Referer: "
#define HTTP_REQ_HEAD_UA                    "User-Agent: "
#define HTTP_REQ_HEAD_CONNECTION            "Connection: "
#define HTTP_REQ_HEAD_ACCEPT                "Accept: "
#define HTTP_REQ_HEAD_ACCEPT_ENCODING       "Accept-Encoding: "
#define HTTP_REQ_HEAD_ACCEPT_CHARSET        "Accept-Charset: "
#define HTTP_REQ_HEAD_ACCEPT_LANGUAGE       "Accept-Language: "
#define HTTP_REQ_HEAD_CONTENT_LENGTH        "Content-Length: "
#define HTTP_REQ_HEAD_CONTENT_TYPE          "Content-Type: "
#define HTTP_REQ_HEAD_COOKIE                "Cookie: "

#define HTTP_RES_HEAD_DATE                  "Date: "
#define HTTP_RES_HEAD_SERVER                "Server: "
#define HTTP_RES_HEAD_CONNECTION            "Connection: "
#define HTTP_RES_HEAD_TRANSFER_ENCODING     "Transfer-Encoding: "
#define HTTP_RES_HEAD_CONTENT_LENGTH        "Content-Length: "
#define HTTP_RES_HEAD_CONTENT_TYPE          "Content-Type: "

extern http_request_t * http_request_get (const char * buffer, http_request_t * request);

extern void http_request_print (http_request_t * request);

extern void http_request_free (http_request_t * request);

extern http_response_t *
http_response_generate (http_response_t * response, enum http_version_t ver, enum http_status_t status,
                        enum http_connection_t connection);

extern void http_response_add_content (http_response_t * response, const char * buffer, int length,
                                       enum http_content_type_t content_type);

extern void http_response_free (http_response_t * response);

/**
 * @return end pointer of buffer */
extern char * http_response_tostring (http_response_t * response, char * buffer);

#endif //__WEB_HTTP_H_
