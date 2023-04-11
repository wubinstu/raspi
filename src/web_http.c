//
// Created by Einc on 2023/03/21.
//

#include "web_http.h"

// HTTP 请求方法
const char * HTTP_REQ_METHOD[] =
        {
                "METHOD_UNKNOWN",
                "GET",
                "POST",
                "HEAD",
                "PUT",
                "DELETE",
                "OPTIONS",
                "TRACE",
                "CONNECT"

        };

/// HTTP 协议版本
const char * HTTP_VERSION[] =
        {
                "VERSION_UNKNOWN",
                "HTTP/0.9",
                "HTTP/1.0",
                "HTTP/1.1",
                "HTTP/2.0"
        };

/// HTTP connection 取值
const char * HTTP_HEAD_CONNECTS[] =
        {
                "connection_unknown",
                "close",
                "Keep-Alive",
                "Upgrade",
                "TE"
        };

/// HTTP 回复报文的状态码 数字状态码: 2n, 状态描述: 2n + 1
const char * HTTP_RES_STATUS[] =
        {
                "0", "UNKNOWN",
                "100", "Continue",                                 // 100
                "101", "Switching Protocols",                      // 101
                "200", "OK",                                       // 200
                "201", "Created",                                  // 201
                "202", "Accepted",                                 // 202
                "203", "Non-Authoritative Information",            // 203
                "204", "No Content",                               // 204
                "205", "Reset Content",                            // 205
                "206", "Partial Content",                          // 206
                "300", "Multiple Choices",                         // 300
                "301", "Moved Permanently",                        // 301
                "302", "Found",                                    // 302
                "303", "See Other",                                // 303
                "304", "Not Modified",                             // 304
                "305", "Use Proxy",                                // 305
                "307", "Temporary Redirect",                       // 307
                "400", "Bad Request",                              // 400
                "401", "Unauthorized",                             // 401
                "402", "Payment Required",                         // 402
                "403", "Forbidden",                                // 403
                "404", "Not Found",                                // 404
                "405", "Method Not Allowed",                       // 405
                "406", "Not Acceptable",                           // 406
                "407", "Proxy Authentication Required",            // 407
                "408", "Request Timeout",                          // 408
                "409", "Conflict",                                 // 409
                "410", "Gone",                                     // 410
                "411", "Length Required",                          // 411
                "412", "Precondition Failed",                      // 412
                "413", "Payload Too Large",                        // 413
                "414", "URI Too Long",                             // 414
                "415", "Unsupported Media Type",                   // 415
                "416", "Range Not Satisfiable",                    // 416
                "417", "Expectation Failed",                       // 417
                "500", "Internal Server Error",                    // 500
                "501", "Not Implemented",                          // 501
                "502", "Bad Gateway",                              // 502
                "503", "Service Unavailable",                      // 503
                "504", "Gateway Timeout",                          // 504
                "505", "HTTP Version Not Supported",               // 505
        };
const char * HTTP_RES_CONTENT_TYPE[] =
        {
                "text/plain",
                "text/html",
                "text/xml",
                "application/json",
                "application/octet-stream",
                "multipart/form-data",
                "image/jpeg",
                "image/png",
                "audio/mpeg",
                "video/mp4",
        };

const char * HTTP_TRANSFER_ENCODING[] =
        {
                "encoding_none",
                "chunked",
                "compress",
                "deflate",
                "gzip",
                "identity",
        };

void http_request_line (http_request_t * request, char ** buffer)
{
    if (request == NULL || buffer == NULL || * buffer == NULL)
        return;

    if (igStrCmp (* buffer, HTTP_REQ_GET, strlen (HTTP_REQ_GET)) == 0)
        request->method = GET, * buffer += (strlen (HTTP_REQ_GET) + 1);

    else if (igStrCmp (* buffer, HTTP_REQ_POST, strlen (HTTP_REQ_POST)) == 0)
        request->method = POST, * buffer += (strlen (HTTP_REQ_POST) + 1);

    else if (igStrCmp (* buffer, HTTP_REQ_HEAD, strlen (HTTP_REQ_HEAD)) == 0)
        request->method = HEAD, * buffer += (strlen (HTTP_REQ_HEAD) + 1);

    else if (igStrCmp (* buffer, HTTP_REQ_PUT, strlen (HTTP_REQ_PUT)) == 0)
        request->method = PUT, * buffer += (strlen (HTTP_REQ_PUT) + 1);

    else if (igStrCmp (* buffer, HTTP_REQ_DELETE, strlen (HTTP_REQ_DELETE)) == 0)
        request->method = DELETE, * buffer += (strlen (HTTP_REQ_DELETE) + 1);

    else if (igStrCmp (* buffer, HTTP_REQ_OPTIONS, strlen (HTTP_REQ_OPTIONS)) == 0)
        request->method = OPTIONS, * buffer += (strlen (HTTP_REQ_OPTIONS) + 1);

    else if (igStrCmp (* buffer, HTTP_REQ_TRACE, strlen (HTTP_REQ_TRACE)) == 0)
        request->method = TRACE, * buffer += (strlen (HTTP_REQ_TRACE) + 1);

    else if (igStrCmp (* buffer, HTTP_REQ_CONNECT, strlen (HTTP_REQ_CONNECT)) == 0)
        request->method = CONNECT, * buffer += (strlen (HTTP_REQ_CONNECT) + 1);

    else
    {
        request->method = METHOD_UNKNOWN;
        return;
    }

    memcpy (request->URI, * buffer, lengthToNextSpace (* buffer));
    * buffer += (lengthToNextSpace (* buffer) + 1);

    if (igStrCmp (* buffer, HTTP_VER_09, strlen (HTTP_VER_09)) == 0)
        request->version = HTTP_09;

    else if (igStrCmp (* buffer, HTTP_VER_10, strlen (HTTP_VER_10)) == 0)
        request->version = HTTP_10;

    else if (igStrCmp (* buffer, HTTP_VER_11, strlen (HTTP_VER_11)) == 0)
        request->version = HTTP_11;

    else if (igStrCmp (* buffer, HTTP_VER_20, strlen (HTTP_VER_20)) == 0)
        request->version = HTTP_20;

    else request->version = VERSION_UNKNOWN;

    moveToNextLine (buffer);

}

void http_request_header (http_request_t * request, char ** buffer)
{
    if (request == NULL || buffer == NULL || * buffer == NULL)
        return;

    do
    {
        if (** buffer == '\n' || (** buffer == '\r' && * (* buffer + 1) == '\n'))
        {
            moveToNextLine (buffer);
            break;
        }

        if (igStrCmp (* buffer, HTTP_REQ_HEAD_HOST, strlen (HTTP_REQ_HEAD_HOST)) == 0)
        {
            * buffer += strlen (HTTP_REQ_HEAD_HOST);
            memcpy (request->host, * buffer, lengthToNextLine (* buffer));
            moveToNextLine (buffer);
            continue;
        }

        if (igStrCmp (* buffer, HTTP_REQ_HEAD_REFERER, strlen (HTTP_REQ_HEAD_REFERER)) == 0)
        {
            * buffer += strlen (HTTP_REQ_HEAD_REFERER);
            memcpy (request->referer, * buffer, lengthToNextLine (* buffer));
            moveToNextLine (buffer);
            continue;
        }

        if (igStrCmp (* buffer, HTTP_REQ_HEAD_UA, strlen (HTTP_REQ_HEAD_UA)) == 0)
        {
            * buffer += strlen (HTTP_REQ_HEAD_UA);
            memcpy (request->userAgent, * buffer, lengthToNextLine (* buffer));
            moveToNextLine (buffer);
            continue;
        }

        if (igStrCmp (* buffer, HTTP_REQ_HEAD_CONNECTION, strlen (HTTP_REQ_HEAD_CONNECTION)) == 0)
        {
            * buffer += strlen (HTTP_REQ_HEAD_CONNECTION);
            if (igStrCmp (* buffer, "Keep-Alive", lengthToNextLine (* buffer)) == 0)
                request->connection = keepalive;
            else if (igStrCmp (* buffer, "close", lengthToNextLine (* buffer)) == 0)
                request->connection = closed;
            else if (igStrCmp (* buffer, "Upgrade", lengthToNextLine (* buffer)) == 0)
                request->connection = upgrade;
            else if (igStrCmp (* buffer, "TE", lengthToNextLine (* buffer)) == 0)
                request->connection = te;
            else request->connection = connection_unknown;
            moveToNextLine (buffer);
            continue;
        }

        if (igStrCmp (* buffer, HTTP_REQ_HEAD_ACCEPT, strlen (HTTP_REQ_HEAD_ACCEPT)) == 0)
        {
            * buffer += strlen (HTTP_REQ_HEAD_ACCEPT);
            memcpy (request->accept, * buffer, lengthToNextLine (* buffer));
            moveToNextLine (buffer);
            continue;
        }

        if (igStrCmp (* buffer, HTTP_REQ_HEAD_ACCEPT_ENCODING, strlen (HTTP_REQ_HEAD_ACCEPT_ENCODING)) == 0)
        {
            * buffer += strlen (HTTP_REQ_HEAD_ACCEPT_ENCODING);
            memcpy (request->acceptEncoding, * buffer, lengthToNextLine (* buffer));
            moveToNextLine (buffer);
            continue;
        }

        if (igStrCmp (* buffer, HTTP_REQ_HEAD_ACCEPT_CHARSET, strlen (HTTP_REQ_HEAD_ACCEPT_CHARSET)) == 0)
        {
            * buffer += strlen (HTTP_REQ_HEAD_ACCEPT_CHARSET);
            memcpy (request->acceptCharset, * buffer, lengthToNextLine (* buffer));
            moveToNextLine (buffer);
            continue;
        }

        if (igStrCmp (* buffer, HTTP_REQ_HEAD_ACCEPT_LANGUAGE, strlen (HTTP_REQ_HEAD_ACCEPT_LANGUAGE)) == 0)
        {
            * buffer += strlen (HTTP_REQ_HEAD_ACCEPT_LANGUAGE);
            memcpy (request->acceptLanguage, * buffer, lengthToNextLine (* buffer));
            moveToNextLine (buffer);
            continue;
        }

        if (igStrCmp (* buffer, HTTP_REQ_HEAD_CONTENT_LENGTH, strlen (HTTP_REQ_HEAD_CONTENT_LENGTH)) == 0)
        {
            * buffer += strlen (HTTP_REQ_HEAD_CONTENT_LENGTH);
            request->contentLength = (int) strtol (* buffer, NULL, 10);
            moveToNextLine (buffer);
            continue;
        }

        if (igStrCmp (* buffer, HTTP_REQ_HEAD_CONTENT_TYPE, strlen (HTTP_REQ_HEAD_CONTENT_TYPE)) == 0)
        {
            * buffer += strlen (HTTP_REQ_HEAD_CONTENT_TYPE);
            memcpy (request->contentType, * buffer, lengthToNextLine (* buffer));
            moveToNextLine (buffer);
            continue;
        }

        if (igStrCmp (* buffer, HTTP_REQ_HEAD_COOKIE, strlen (HTTP_REQ_HEAD_COOKIE)) == 0)
        {
            * buffer += strlen (HTTP_REQ_HEAD_COOKIE);
            memcpy (request->cookie, * buffer, lengthToNextLine (* buffer));
            moveToNextLine (buffer);
            continue;
        }

        moveToNextLine (buffer);

    } while (true);
}

void http_request_body (http_request_t * request, char ** buffer)
{
    if (request == NULL || buffer == NULL || * buffer == NULL)
        return;

    if (request->contentLength > 0)
    {
        request->body = calloc (1, request->contentLength);
        if (request->body != NULL)
            memcpy (request->body, * buffer, request->contentLength);
        * buffer += request->contentLength;
    }
}

void http_request_free (http_request_t * request)
{
    if (request != NULL)
    {
        if (request->body != NULL)
            free (request->body);
        free (request);
    }
}

void http_request_print (http_request_t * request)
{
    if (request == NULL)
        return;

    printf ("\r\n%s %s %s\r\n", HTTP_REQ_METHOD[request->method], request->URI, HTTP_VERSION[request->version]);
    if (request->host[0] != '\0')
        printf ("%s%s\r\n", HTTP_REQ_HEAD_HOST, request->host);
    if (request->referer[0] != '\0')
        printf ("%s%s\r\n", HTTP_REQ_HEAD_REFERER, request->referer);
    if (request->userAgent[0] != '\0')
        printf ("%s%s\r\n", HTTP_REQ_HEAD_UA, request->userAgent);
    if (request->connection != connection_unknown)
        printf ("%s%s\r\n", HTTP_REQ_HEAD_CONNECTION, HTTP_HEAD_CONNECTS[request->connection]);
    if (request->accept[0] != '\0')
        printf ("%s%s\r\n", HTTP_REQ_HEAD_ACCEPT, request->accept);
    if (request->acceptLanguage[0] != '\0')
        printf ("%s%s\r\n", HTTP_REQ_HEAD_ACCEPT_LANGUAGE, request->acceptLanguage);
    if (request->acceptCharset[0] != '\0')
        printf ("%s%s\r\n", HTTP_REQ_HEAD_ACCEPT_CHARSET, request->acceptCharset);
    if (request->acceptEncoding[0] != '\0')
        printf ("%s%s\r\n", HTTP_REQ_HEAD_ACCEPT_ENCODING, request->acceptEncoding);
    if (request->contentLength != '\0')
        printf ("%s%d\r\n", HTTP_REQ_HEAD_CONTENT_LENGTH, request->contentLength);
    if (request->contentType[0] != '\0')
        printf ("%s%s\r\n", HTTP_REQ_HEAD_CONTENT_TYPE, request->contentType);
    if (request->cookie[0] != '\0')
        printf ("%s%s\r\n", HTTP_REQ_HEAD_COOKIE, request->cookie);
    if (request->contentLength > 0 && request->body != NULL)
        printf ("\r\n%s", request->body);
    printf ("\r\n");
}

http_request_t * http_request_get (const char * buffer, http_request_t * request)
{
    if (buffer == NULL)
        return NULL;

    if (request == NULL)
        request = calloc (1, sizeof (http_request_t));

    char * location = (char *) buffer;

    http_request_line (request, & location);
    http_request_header (request, & location);
//    http_request_body (request, & location);

    return request;
}

http_response_t *
http_response_generate (http_response_t * response, enum http_version_t ver, enum http_status_t status,
                        enum http_connection_t connection)
{

    if (response == NULL)
        response = (http_response_t *) calloc (1, sizeof (http_response_t));
    memset (response, 0, sizeof (http_response_t));
    response->version = ver;
    response->status = status;
    response->encoding = encoding_none;
    response->connection = connection;
    response->contentLength = 0;
    response->body = NULL;
    memcpy (response->server, PROJECT_SERVER_NAME, sizeof (PROJECT_SERVER_NAME));
    time_t now = time (NULL);
    struct tm tp;
    localtime_r (& now, & tp);
    strftime (response->date, 40, "Date: %a, %d %b %Y %H:%M:%S GMT", & tp);
    return response;
}

void http_response_add_content (http_response_t * response, const char * buffer, int length,
                                enum http_content_type_t content_type)
{
    if (response == NULL || buffer == NULL || length <= 0)
        return;

//    response->body = calloc (1, length);
//    if (response->body == NULL)
//        return;
//    memcpy (response->body, buffer, length);
    response->body = (char *) buffer;
    response->contentLength = length;
    response->contentType = content_type;
}

void http_response_free (http_response_t * response)
{
    if (response == NULL)
        return;

    if (response->contentLength > 0 && response->body != NULL)
        free (response->body);

    free (response);
}

char * http_response_tostring (http_response_t * response, char * buffer)
{
    if (response == NULL || buffer == NULL)
        return NULL;

    char buf[20000] = {0};

    sprintf (buffer, "%s %d %s\r\n", HTTP_VERSION[response->version],
             (int) strtol (HTTP_RES_STATUS[2 * response->status], NULL, 10),
             HTTP_RES_STATUS[2 * response->status + 1]);

    sprintf (buf, "%s%s\r\n", HTTP_RES_HEAD_DATE, response->date);
    strcat (buffer, buf);

    memset (buf, 0, sizeof (buf));
    sprintf (buf, "%s%s\r\n", HTTP_RES_HEAD_SERVER, response->server);
    strcat (buffer, buf);

    memset (buf, 0, sizeof (buf));
    sprintf (buf, "%s%s\r\n", HTTP_RES_HEAD_CONNECTION, HTTP_HEAD_CONNECTS[response->connection]);
    strcat (buffer, buf);

    if (response->encoding != encoding_none)
    {
        memset (buf, 0, sizeof (buf));
        sprintf (buf, "%s%s\r\n", HTTP_RES_HEAD_TRANSFER_ENCODING, HTTP_TRANSFER_ENCODING[response->encoding]);
        strcat (buffer, buf);
    }

    char * loc = buffer + strlen (buffer);

    if (response->contentLength > 0)
    {
        memset (buf, 0, sizeof (buf));
        sprintf (buf, "%s%d\r\n", HTTP_RES_HEAD_CONTENT_LENGTH, response->contentLength);
        strcat (buffer, buf);

        memset (buf, 0, sizeof (buf));
        sprintf (buf, "%s%s\r\n", HTTP_RES_HEAD_CONTENT_TYPE, HTTP_RES_CONTENT_TYPE[response->contentType]);
        strcat (buffer, buf);

        loc = buffer + strlen (buffer);
        if (response->body != NULL)
        {
            strcat (buffer, "\r\n");
            loc = buffer + strlen (buffer);
            memcpy (loc, response->body, response->contentLength);
            loc += response->contentLength;
        }
    }

    return loc;
}
