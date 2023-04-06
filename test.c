//
// Created by Einc on 2023/03/31.
//
#include "types.h"
#include "web_http.h"
#include "global.h"
#include "filed.h"

int main ()
{

    char req[] = "POST /api/login HTTP/1.1\r\n"
                 "Host: example.com\r\n"
                 "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:98.0) Gecko/20100101 Firefox/98.0\r\n"
                 "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
                 "Accept-Language: en-US,en;q=0.5\r\n"
                 "Accept-Encoding: gzip, deflate, br\r\n"
                 "Content-Type: application/x-www-form-urlencoded\r\n"
                 "Content-Length: 27\r\n"
                 "Connection: keep-alive\r\n"
                 "Referer: https://example.com/login\r\n"
                 "Cookie: sessionid=123456789\r\n"
                 "\r\n"
                 "username=johndoe&password=1234";

    char resp[] = "HTTP/1.1 200 OK\r\n"
                  "Date: Thu, 31 Mar 2023 10:30:00 GMT\r\n"
                  "Server: Apache/2.4.43 (Unix) OpenSSL/1.1.1d PHP/7.4.6\r\n"
                  "Last-Modified: Mon, 28 Mar 2023 15:22:36 GMT\r\n"
                  "ETag: \"25-5be51430d15c0\"\r\n"
                  "Accept-Ranges: bytes\r\n"
                  "Content-Length: 37\r\n"
                  "Keep-Alive: timeout=5, max=100\r\n"
                  "Connection: Keep-Alive\r\n"
                  "Content-Type: text/plain\r\n"
                  "\r\n"
                  "Hello World! This is an HTTP response.";

//    http_request_t * r = http_request_get (req);
//    http_request_print (r);
//    http_request_free (r);
//
//    http_response_t * res = http_response_generate (HTTP_11, HTTP_200, keepalive);

//    http_response_add_content (res, "HelloClient!", 12, text_plain);
//    char buf[1000];
//    http_response_tostring (res, buf);
//    putchar ('\n');
//    putchar ('\n');
//    puts (buf);
//    http_response_free (res);

//    char buf[1024 * 1024 * 10] = {0};
//    char * buf = calloc (10, 1024 * 1024);

//    web_html_fd = readOpen (WEB_HTML_PAGE);
//    web_html_bg_image_fd = readOpen (WEB_HTML_BG);
//    web_html_size = fileSize (WEB_HTML_PAGE);
//    web_html_bg_image_size = fileSize (WEB_HTML_BG);
//
//    web_html_buf = mmap (NULL, web_html_size, PROT_READ, MAP_PRIVATE, web_html_fd, 0);
//    web_html_bg_image_buf = mmap (NULL, web_html_bg_image_size, PROT_READ, MAP_PRIVATE, web_html_bg_image_fd, 0);
//    http_response_t * res = http_response_generate (HTTP_11, HTTP_200, keepalive);
//    http_response_add_content (res, web_html_bg_image_buf, (int) web_html_bg_image_size, image_jpeg);
//    http_response_tostring (res, buf);

    hash_table_info_raspi_http = hash_table_info_init (HASH_TABLE_SIZE);
    hash_node_sql_data_t abc;
    abc.socket_fd = 13;
    abc.next = NULL;
    struct timespec t;
    clock_gettime (CLOCK_REALTIME, & t);
    t.tv_sec += 5;
    hash_table_info_update (hash_table_info_raspi_http, & abc, t);
    abc.socket_fd = 14;
    hash_table_info_update (hash_table_info_raspi_http, & abc, t);
    hash_table_info_update (hash_table_info_raspi_http, & abc, t);

    hash_table_info_delete (hash_table_info_raspi_http, 13);
    hash_table_info_delete (hash_table_info_raspi_http, 14);
    hash_table_info_delete (hash_table_info_raspi_http, 14);


    return 0;
}