#ifndef _LWS_HTTP_PLUGIN_H_
#define _LWS_HTTP_PLUGIN_H_

extern int lws_default_handler(lws_http_conn_t *c, int ev, void *p);
extern int lws_hello_handler(lws_http_conn_t *c, int ev, void *p);
extern int lws_version_handler(lws_http_conn_t *c, int ev, void *p);
extern int lws_download_handler(lws_http_conn_t *c, int ev, void *p);

#endif // _LWS_HTTP_PLUGIN_H_

