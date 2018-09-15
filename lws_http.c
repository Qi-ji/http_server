#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "lws_log.h"
#include "lws_http.h"

typedef struct _lws_http_status_t {
    int http_code;
    char *http_status;
} lws_http_status_t;

/* Status Codes */
static lws_http_status_t lws_http_status[] = {
    {100,    "Continue"},
    {101,    "SwitchingProtocols"},
    {102,    "Processing"},
    {200,    "OK"},
    {201,    "Created"},
    {202,    "Accepted"},
    {203,    "Non-AuthoritativeInformation"},
    {204,    "NoContent"},
    {205,    "ResetContent"},
    {206,    "PartialContent"},
    {207,    "Multi-Status"},
    {208,    "AlreadyReported"},
    {226,    "IMUsed"},
    {300,    "MultipleChoices"},
    {301,    "MovedPermanently"},
    {302,    "Found"},
    {303,    "SeeOther"},
    {304,    "NotModified"},
    {305,    "UseProxy"},
    {307,    "TemporaryRedirect"},
    {308,    "PermanentRedirect"},
    {400,    "BadRequest"},
    {401,    "Unauthorized"},
    {402,    "PaymentRequired"},
    {403,    "Forbidden"},
    {404,    "NotFound"},
    {405,    "MethodNotAllowed"},
    {406,    "NotAcceptable"},
    {407,    "ProxyAuthenticationRequired"},
    {408,    "RequestTimeout"},
    {409,    "Conflict"},
    {410,    "Gone"},
    {411,    "LengthRequired"},
    {412,    "PreconditionFailed"},
    {413,    "PayloadTooLarge"},
    {414,    "URITooLong"},
    {415,    "UnsupportedMediaType"},
    {416,    "RangeNotSatisfiable"},
    {417,    "ExpectationFailed"},
    {421,    "MisdirectedRequest"},
    {422,    "UnprocessableEntity"},
    {423,    "Locked"},
    {424,    "FailedDependency"},
    {426,    "UpgradeRequired"},
    {428,    "PreconditionRequired"},
    {429,    "TooManyRequests"},
    {431,    "RequestHeaderFieldsTooLarge"},
    {451,    "UnavailableForLegalReasons"},
    {500,    "InternalServerError"},
    {501,    "NotImplemented"},
    {502,    "BadGateway"},
    {503,    "ServiceUnavailable"},
    {504,    "GatewayTimeout"},
    {505,    "HTTPVersionNotSupported"},
    {506,    "VariantAlsoNegotiates"},
    {507,    "InsufficientStorage"},
    {508,    "LoopDetected"},
    {510,    "NotExtended"},
    {511,    "NetworkAuthenticationRequired"},
};

const char *lws_skip(const char *s, const char *end, const char *delims, struct lws_str *v)
{
    v->p = s;
    while (s < end && strchr(delims, *(unsigned char *) s) == NULL) s++;
    v->len = s - v->p;
    while (s < end && strchr(delims, *(unsigned char *) s) != NULL) s++;
    return s;
}

/*
 * Check whether full request is buffered. Return:
 *   -1  if request is malformed
 *    0  if request is not yet fully buffered
 *   >0  actual request length, including last \r\n\r\n
 */
static int lws_http_get_request_len(const char *s, int buf_len)
{
    const unsigned char *buf = (unsigned char *)s;
    int i;

    for (i = 0; i < buf_len; i++) {
        if (!isprint(buf[i]) && buf[i] != '\r' && buf[i] != '\n' && buf[i] < 128) {
            return -1;
        } else if (buf[i] == '\n' && i + 1 < buf_len && buf[i + 1] == '\n') {
            return i + 2;
        } else if (buf[i] == '\n' && i + 2 < buf_len && buf[i + 1] == '\r' &&
            buf[i + 2] == '\n') {
            return i + 3;
        }
    }

    return 0;
}

static const char *lws_http_parse_headers(const char *s, const char *end, int len, struct http_message *req)
{
    int i;

    for (i = 0; i < (int) ARRAY_SIZE(req->header_names) - 1; i++) {
        struct lws_str *k = &req->header_names[i], *v = &req->header_values[i];

        s = lws_skip(s, end, ": ", k);
        s = lws_skip(s, end, "\r\n", v);

        while (v->len > 0 && v->p[v->len - 1] == ' ') {
            v->len--; /* Trim trailing spaces in header value */
        }

        if (k->len == 0 || v->len == 0) {
            k->p = v->p = NULL;
            k->len = v->len = 0;
            break;
        }

        if (!strncasecmp(k->p, "Content-Length", 14)) {
            req->body.len = atoi(v->p);
            req->message.len = len + req->body.len;
        }
    }

    return s;
}

int lws_parse_http(const char *s, int n, struct http_message *hm, int is_req)
{
    const char *end, *qs;
    int len = lws_http_get_request_len(s, n);

    lws_log(4, "get request len: %d\n", len);
    if (len <= 0) return len;

    memset(hm, 0, sizeof(struct http_message));
    hm->message.p = s;
    hm->body.p = s + len;
    hm->message.len = hm->body.len = (size_t) ~0;
    end = s + len;

    /* Request is fully buffered. Skip leading whitespaces. */
    while (s < end && isspace(*(unsigned char *) s)) s++;

    if (is_req) {
        /* Parse request line: method, URI, proto */
        s = lws_skip(s, end, " ", &hm->method);
        s = lws_skip(s, end, " ", &hm->uri);
        s = lws_skip(s, end, "\r\n", &hm->proto);
        if (hm->uri.p <= hm->method.p || hm->proto.p <= hm->uri.p) return -1;

        /* If URI contains '?' character, initialize query_string */
        if ((qs = (char *) memchr(hm->uri.p, '?', hm->uri.len)) != NULL) {
            hm->query_string.p = qs + 1;
            hm->query_string.len = &hm->uri.p[hm->uri.len] - (qs + 1);
            hm->uri.len = qs - hm->uri.p;
        }
    } else {
        s = lws_skip(s, end, " ", &hm->proto);
        if (end - s < 4 || s[3] != ' ') return -1;
        hm->resp_code = atoi(s);
        if (hm->resp_code < 100 || hm->resp_code >= 600) return -1;
        s += 4;
        s = lws_skip(s, end, "\r\n", &hm->resp_status_msg);
    }

    s = lws_http_parse_headers(s, end, len, hm);

    /*
    * lws_parse_http() is used to parse both HTTP requests and HTTP
    * responses. If HTTP response does not have Content-Length set, then
    * body is read until socket is closed, i.e. body.len is infinite (~0).
    *
    * For HTTP requests though, according to
    * http://tools.ietf.org/html/rfc7231#section-8.1.3,
    * only POST and PUT methods have defined body semantics.
    * Therefore, if Content-Length is not specified and methods are
    * not one of PUT or POST, set body length to 0.
    *
    * So,
    * if it is HTTP request, and Content-Length is not set,
    * and method is not (PUT or POST) then reset body length to zero.
    */
    if (hm->body.len == (size_t) ~0 && is_req &&
        strcmp(hm->method.p, "PUT") != 0 &&
        strcmp(hm->method.p, "POST") != 0) {
        hm->body.len = 0;
        hm->message.len = len;
    }

    return len;
}

/**
 * http protocol interfaces
**/
struct lws_str *lws_get_http_header(struct http_message *hm, const char *name)
{
    size_t i, len = strlen(name);

    for (i = 0; hm->header_names[i].len > 0; i++) {
        struct lws_str *h = &hm->header_names[i], *v = &hm->header_values[i];
        if (h->p != NULL && h->len == len && !strncasecmp(h->p, name, len))
            return v;
    }

    return NULL;
}

static char *lws_get_http_status(int http_code)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(lws_http_status); i++) {
        if (http_code == lws_http_status[i].http_code) {
            return lws_http_status[i].http_status;
        }
    }

    return "unknow";
}

/**
 * http response interfaces
**/
int lws_http_respond_base(lws_http_conn_t *lws_http_conn, int http_code, char *content_type, 
                          char *extra_headers, int close_flag, char *content, int content_length)
{
    int header_length = lws_http_conn->send_length;
    char *send_buf = lws_http_conn->send_buf;
    int send_length = 0;

    if (lws_http_conn->send == NULL)
        return -1;

    /* HTTP/1.1 */
    header_length += sprintf(send_buf + header_length, "%s %d %s\r\n", LWS_HTTP_PROTO, http_code, lws_get_http_status(http_code));
    header_length += sprintf(send_buf + header_length, "Host: %s %s\r\n", LWS_HTTP_HOST, LWS_HTTP_VERSION);

    if (content_length < 0) {
        header_length += sprintf(send_buf + header_length, "%s", "Transfer-Encoding: chunked\r\n");
    } else {
        header_length += sprintf(send_buf + header_length, "Content-Length: %d\r\n", content_length);
    }

    if (content_type) {
        header_length += sprintf(send_buf + header_length, "Content-Type: %s\r\n", content_type);
    }

    if (extra_headers) {
        header_length += sprintf(send_buf + header_length, "%s\r\n", extra_headers);
    }

    if (close_flag) {
        header_length += sprintf(send_buf + header_length, "Connection: %s\r\n", "close");
    } else {
        header_length += sprintf(send_buf + header_length, "Connection: %s\r\n", "keep-alive");
    }

    /* "\r\n\r\n" */
    header_length += sprintf(send_buf + header_length, "%s", "\r\n");
    lws_http_conn->send_length = header_length;

    /* send header */
    lws_log(4, "Send: %.*s\n", lws_http_conn->send_length, lws_http_conn->send_buf);
    send_length += lws_http_conn->send(lws_http_conn->sockfd, lws_http_conn->send_buf, lws_http_conn->send_length);
    if (send_length <= 0) {
        return -1;
    } else {
        lws_http_conn->send_length = 0;
    }

    if (content && content_length > 0) {
        lws_log(4, "Send body_size: %d\n", content_length);
        send_length += lws_http_conn->send(lws_http_conn->sockfd, content, content_length);
    }

    return send_length;
}

int lws_http_respond(lws_http_conn_t *lws_http_conn, int http_code, int close_flag, 
                     char *content_type, char *content, int content_length)
{
    return lws_http_respond_base(lws_http_conn, http_code, content_type, NULL, close_flag, content, content_length);
}

int lws_http_respond_header(lws_http_conn_t *lws_http_conn, int http_code, int close_flag)
{
    return lws_http_respond_base(lws_http_conn, http_code, LWS_HTTP_HTML_TYPE, NULL, close_flag, NULL, 0);
}

/**
 * http plugin interfaces
**/
static lws_http_plugins_t lws_http_plugins = {NULL, NULL, 0, NULL};

lws_event_handler_t lws_http_get_endpoint_hander(const char *uri, int uri_size)
{
    lws_http_plugins_t *plugin;
    lws_event_handler_t hander = NULL;

    if (uri == NULL || uri_size <= 0)
        return NULL;

    plugin = &lws_http_plugins;
    while (plugin && plugin->uri) {
        if (uri_size == plugin->uri_size &&
            strncmp(plugin->uri, uri, uri_size) == 0) {
            hander = plugin->handler;
            break;
        }

        plugin = plugin->next;
    }

    return hander;
}

void lws_http_endpoint_register(const char *uri, int uri_size, lws_event_handler_t handler)
{
    lws_http_plugins_t *plugin;
    lws_http_plugins_t *new_plugin;

    if (uri == NULL || uri_size <= 0 || handler == NULL)
        return ;

    plugin = &lws_http_plugins;
    while (plugin) {
        if (plugin->handler == NULL) {
            new_plugin = plugin;
            break;
        }

        if (plugin->next == NULL) {
            new_plugin = malloc(sizeof(lws_http_plugins_t));
            plugin->next = new_plugin;
            break;
        }

        plugin = plugin->next;
    }

    /* insert list */
    if (new_plugin) {
        new_plugin->handler = handler;
        new_plugin->uri_size = uri_size;
        new_plugin->uri = strndup(uri, uri_size);
        new_plugin->next = NULL;
        lws_log(3, "register endpoint: %.*s\n", uri_size, uri);
    }
}

/**
 * http connection interfaces
**/
void lws_http_conn_print(struct http_message *hm)
{
    int i;

    lws_log(4, "http request: %.*s %.*s %.*s\n", hm->method.len, hm->method.p,
                hm->uri.len, hm->uri.p, hm->proto.len, hm->proto.p);
    for (i = 0; i < LWS_MAX_HTTP_HEADERS; i++) {
        if (hm->header_names[i].len > 0 && hm->header_values[i].len > 0) {
            lws_log(4, "http header: %.*s: %.*s\n", hm->header_names[i].len, hm->header_names[i].p, 
                        hm->header_values[i].len, hm->header_values[i].p);
        }
    }
}

lws_http_conn_t *lws_http_conn_init(int sockfd)
{
    lws_http_conn_t *lws_http_conn;

    lws_http_conn = (lws_http_conn_t *)malloc(sizeof(lws_http_conn_t));
    if (lws_http_conn == NULL)
        return NULL;

    lws_http_conn->sockfd = sockfd;
    lws_http_conn->send = NULL;
    lws_http_conn->send_length = 0;
    lws_http_conn->recv_length = 0;
    return lws_http_conn;
}

int lws_http_conn_exit(lws_http_conn_t *lws_http_conn)
{
    if (lws_http_conn)
        free(lws_http_conn);

    return 0;
}

int lws_http_conn_recv(lws_http_conn_t *lws_http_conn, char *data, size_t size)
{
    struct http_message http_msg;
    lws_event_handler_t handler;
    struct lws_str *connect;
    int len = 0;

    if (lws_http_conn == NULL)
        return -1;

    lws_log(4, "start lws_parse_http size: %d\n", size);
    len = lws_parse_http(data, size, &http_msg, 1);
    if (len < 0) {
        lws_log(2, "lws_parse_http failed, len: %d\n", len);
        return -1;
    }

    /* print http data */
    lws_log(4, "lws_parse_http len: %d\n", len);
    lws_http_conn_print(&http_msg);

    /* parse Connection */
    connect = lws_get_http_header(&http_msg, "Connection");
    if (strncasecmp(connect->p, "close", connect->len) == 0) {
        lws_http_conn->close_flag = 1;
    }

    handler = lws_http_get_endpoint_hander(http_msg.uri.p, http_msg.uri.len);
    if (handler) {
        handler(lws_http_conn, LWS_EV_HTTP_REQUEST, (void *)&http_msg);
    } else {
        lws_log(2, "Not found uri: %.*s\n", http_msg.uri.len, http_msg.uri.p);
        lws_http_respond_header(lws_http_conn, 404, lws_http_conn->close_flag);
    }

    return len;
}

