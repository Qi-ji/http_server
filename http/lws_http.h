#ifndef _LWS_HTTP_H_
#define _LWS_HTTP_H_

#ifndef LWS_MAX_HTTP_HEADERS
#define LWS_MAX_HTTP_HEADERS    20
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#endif

#define LWS_HTTP_PROTO          "HTTP/1.1"
#define LWS_HTTP_HOST           "LWS"
#define LWS_HTTP_VERSION        "1.0.1"

/* Content-Type */
#define LWS_HTTP_HTML_TYPE      "text/html"
#define LWS_HTTP_PLAIN_TYPE     "text/plain"
#define LWS_HTTP_XML_TYPE       "application/xml"
#define LWS_HTTP_JSON_TYPE      "application/json"
#define LWS_HTTP_PDF_TYPE       "application/pdf"
#define LWS_HTTP_OCTET_STREAM   "application/octet-stream"
#define LWS_HTTP_JPEG_TYPE      "image/jpeg"
#define LWS_HTTP_BMP_TYPE       "image/bmp"
#define LWS_HTTP_PNG_TYPE       "image/png"
#define LWS_HTTP_GIF_TYPE       "image/gif"
#define LWS_HTTP_MPEG_TYPE      "video/mpeg"
#define LWS_HTTP_MP4_TYPE       "video/mp4"

/* HTTP and websocket events. void *ev_data is described in a comment. */
#define LWS_EV_HTTP_REQUEST     100 /* struct http_message * */
#define LWS_EV_HTTP_REPLY       101   /* struct http_message * */
#define LWS_EV_HTTP_CHUNK       102   /* struct http_message * */
#define LWS_EV_SSI_CALL         105     /* char * */

/* HTTP response status codes */
#define HTTP_CONTINUE                       100
#define HTTP_SWITCHING_PROCOTOLS            101
#define HTTP_PROCESSING                     102
#define HTTP_OK                             200
#define HTTP_CREATED                        201
#define HTTP_ACCEPTED                       202
#define HTTP_NON_AUTHORATATIVE              203
#define HTTP_NO_CONTENT                     204
#define HTTP_RESET_CONTENT                  205
#define HTTP_PARTIAL_CONTENT                206
#define HTTP_MULTIPLE_CHOICES               300
#define HTTP_MOVED_PERMANENTLY              301
#define HTTP_FOUND                          302
#define HTTP_SEE_OTHER                      303
#define HTTP_NOT_MODIFIED                   304
#define HTTP_USE_PROXY                      305
#define HTTP_TEMPORARY_REDIRECT             307
#define HTTP_BAD_REQUEST                    400
#define HTTP_UNAUTHORIZED                   401
#define HTTP_PAYMENT_REQD                   402
#define HTTP_FORBIDDEN                      403
#define HTTP_NOT_FOUND                      404
#define HTTP_METHOD_NOT_ALLOWED             405
#define HTTP_NOT_ACCEPTABLE                 406
#define HTTP_PROXY_AUTH_REQD                407
#define HTTP_REQUEST_TIMEOUT                408
#define HTTP_CONFLICT                       409
#define HTTP_GONE                           410
#define HTTP_LENGTH_REQUIRED                411
#define HTTP_PRECONDITION_FAILED            412
#define HTTP_REQ_ENTITY_TOO_LARGE           413
#define HTTP_REQ_URI_TOO_LONG               414
#define HTTP_UNSUPPORTED_MEDIA_TYPE         415
#define HTTP_REQUEST_RANGE_NOT_SATISFIABLE  416
#define HTTP_EXPECTATION_FAILED             417
#define HTTP_INTERNAL_SERVER_ERROR          500
#define HTTP_NOT_IMPLEMENTED                501
#define HTTP_BAD_GATEWAY                    502
#define HTTP_SERVICE_UNAVAILABLE            503
#define HTTP_GATEWAY_TIMEOUT                504
#define HTTP_HTTP_VERSION_NOT_SUPPORTED     505

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

/**
 * http connection interfaces
**/
typedef struct _lws_http_conn_t_ {
    int sockfd;
    int close_flag;
    char recv_buf[4096];
    int recv_length;
    char send_buf[4096];
    int send_length;
    int (*send)(int sockfd, char *data, int size);
    int (*recv)(int sockfd, char *data, int *size);
    int (*close)(int sockfd);
} lws_http_conn_t;

extern lws_http_conn_t *lws_http_conn_init(int sockfd);
extern int lws_http_conn_exit(lws_http_conn_t *lws_http_conn);
extern int lws_http_conn_recv(lws_http_conn_t *lws_http_conn, char *data, size_t size);

/**
 * http protocol interfaces
**/
extern struct lws_str *lws_get_http_header(struct http_message *hm, const char *name);

/**
 * http response interfaces
**/
extern int lws_http_respond_base(lws_http_conn_t *lws_http_conn, int http_code, char *content_type, 
                          char *extra_headers, int close_flag, char *content, int content_length);
extern int lws_http_respond(lws_http_conn_t *lws_http_conn, int http_code, int close_flag, 
                     char *content_type, char *content, int content_length);
extern int lws_http_respond_header(lws_http_conn_t *lws_http_conn, int http_code, int close_flag);

/**
 * http plugin interfaces
**/
typedef int (*lws_event_handler_t)(lws_http_conn_t *c, int ev, void *p);

typedef struct lws_http_plugins_t {
    struct lws_http_plugins_t *next;
    const char *uri;
    size_t uri_size;
    lws_event_handler_t handler;
} lws_http_plugins_t;

extern lws_event_handler_t lws_http_get_endpoint_hander(const char *uri, int uri_size);
extern void lws_http_endpoint_register(const char *uri, int uri_size, lws_event_handler_t handler);
extern char *lws_http_contenttype(char *filename);

#endif // _LWS_HTTP_H_

