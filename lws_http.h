#ifndef _LWS_HTTP_H_
#define _LWS_HTTP_H_

#ifndef LWS_MAX_HTTP_HEADERS
#define LWS_MAX_HTTP_HEADERS    20
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#endif

/* HTTP and websocket events. void *ev_data is described in a comment. */
#define LWS_EV_HTTP_REQUEST     100 /* struct http_message * */
#define LWS_EV_HTTP_REPLY       101   /* struct http_message * */
#define LWS_EV_HTTP_CHUNK       102   /* struct http_message * */
#define LWS_EV_SSI_CALL         105     /* char * */

/* Describes chunk of memory */
struct lws_str {
  const char *p; /* Memory chunk pointer */
  size_t len;    /* Memory chunk length */
};

/* HTTP message */
struct http_message {
  struct lws_str message; /* Whole message: request line + headers + body */

  /* HTTP Request line (or HTTP response line) */
  struct lws_str method; /* "GET" */
  struct lws_str uri;    /* "/my_file.html" */
  struct lws_str proto;  /* "HTTP/1.1" -- for both request and response */

  /* For responses, code and response status message are set */
  int resp_code;
  struct lws_str resp_status_msg;

  /*
   * Query-string part of the URI. For example, for HTTP request
   *    GET /foo/bar?param1=val1&param2=val2
   *    |    uri    |     query_string     |
   *
   * Note that question mark character doesn't belong neither to the uri,
   * nor to the query_string
   */
  struct lws_str query_string;

  /* Headers */
  struct lws_str header_names[LWS_MAX_HTTP_HEADERS];
  struct lws_str header_values[LWS_MAX_HTTP_HEADERS];

  /* Message body */
  struct lws_str body; /* Zero-length for requests with no body */
};

typedef struct _lws_http_conn_t_ {
    int sockfd;
    int close_flag;
    int (*send)(int sockfd, char *data, int size);
} lws_http_conn_t;

/* http connection */
extern lws_http_conn_t *lws_http_conn_init(int sockfd);
extern int lws_http_conn_exit(lws_http_conn_t *lws_http_conn);
extern int lws_http_conn_recv(lws_http_conn_t *lws_http_conn, char *data, size_t size);

/* http message */
extern struct lws_str *lws_get_http_header(struct http_message *hm, const char *name);

#endif // _LWS_HTTP_H_

