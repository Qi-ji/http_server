#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "lws_log.h"
#include "lws_http.h"
#include "lws_http_plugin.h"

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

typedef void (*lws_event_handler_t)(lws_http_conn_t *c, int ev, void *p);

typedef struct lws_http_plugins_t {
    const char *endpoint;
    lws_event_handler_t handler;
} lws_http_plugins_t;

lws_http_plugins_t lws_http_plugins[] = {
    {"/",           lws_default_handler},
    {"/hello",      lws_hello_handler},

    {NULL,          NULL}
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

lws_http_conn_t *lws_http_conn_init(int sockfd)
{
    lws_http_conn_t *lws_http_conn;

    lws_http_conn = (lws_http_conn_t *)malloc(sizeof(lws_http_conn_t));
    if (lws_http_conn == NULL)
        return NULL;

    lws_http_conn->sockfd = sockfd;
    lws_http_conn->send = NULL;
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
    struct http_message http_msg, *hm;
    lws_http_plugins_t *plugin;
    int len = 0;
    int i;

    if (lws_http_conn == NULL)
        return -1;

    lws_log(4, "start lws_parse_http size: %d\n", size);
    len = lws_parse_http(data, size, &http_msg, 1);
    if (len < 0) {
        lws_log(2, "lws_parse_http failed, len: %d\n", len);
        return -1;
    }

    hm = &http_msg;
    lws_log(4, "lws_parse_http len: %d\n", len);
    lws_log(4, "http request: %.*s %.*s %.*s\n", hm->method.len, hm->method.p,
                hm->uri.len, hm->uri.p, hm->proto.len, hm->proto.p);
    for (i = 0; i < LWS_MAX_HTTP_HEADERS; i++) {
        if (hm->header_names[i].len > 0 && hm->header_values[i].len > 0) {
            lws_log(4, "http header: %.*s: %.*s\n", hm->header_names[i].len, hm->header_names[i].p, 
                        hm->header_values[i].len, hm->header_values[i].p);
        }
    }

    /* see plugin table */
    plugin = lws_http_plugins;
    while (plugin && plugin->endpoint) {
        if (strncmp(plugin->endpoint, hm->uri.p, hm->uri.len) == 0) {
            plugin->handler(lws_http_conn, LWS_EV_HTTP_REQUEST, (void *)&http_msg);
            break;
        }

        plugin++;
    }

    if (plugin == NULL || plugin->endpoint == NULL) {
        lws_log(2, "404 NOT FOUND\n");
        lws_notfound_handler(lws_http_conn, LWS_EV_HTTP_REQUEST, NULL);
    }

    return len;
}

