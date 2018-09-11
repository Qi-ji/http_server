#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "lws_http.h"
#include "lws_http_plugin.h"

void lws_default_handler(lws_http_conn_t *c, int ev, void *p)
{
    struct http_message *hm = p;
    char data[1024] = {0};

    if (hm && ev == LWS_EV_HTTP_REQUEST) {
        if (c->send == NULL) {
            c->close_flag = 1;
            return ;
        }

        sprintf(data, "%s",
                  "<html><body><h>Enjoy your webserver!</h><br/><br/>"
                  "<ul style=\"list-style-type:circle\">"
                  "<li><a href=\"/hello\"> echo hello message </a></li>"
                  "<li><a href=\"/version\"> echo lws version </a></li>"
                  "</ul>"
                  "</body></html>");

        lws_http_respond(c, 200, LWS_HTTP_HTML_TYPE, data, strlen(data));
        c->close_flag = 1;
    }
}

void lws_hello_handler(lws_http_conn_t *c, int ev, void *p)
{
    struct http_message *hm = p;
    char data[1024] = {0};

    if (hm && ev == LWS_EV_HTTP_REQUEST) {
        if (c->send == NULL) {
            c->close_flag = 1;
            return ;
        }

        sprintf(data, "%s",
                  "<html><body><h>Hello LWS!</h><br/><br/>"
                  "</body></html>");

        lws_http_respond(c, 200, LWS_HTTP_HTML_TYPE, data, strlen(data));
        c->close_flag = 1;
    }
}

void lws_version_handler(lws_http_conn_t *c, int ev, void *p)
{
    struct http_message *hm = p;
    char data[1024] = {0};

    if (hm && ev == LWS_EV_HTTP_REQUEST) {
        if (c->send == NULL) {
            c->close_flag = 1;
            return ;
        }

        sprintf(data, "%s",
                  "<html><body><h>LWS - version[1.0.0]</h><br/><br/>"
                  "</body></html>");

        lws_http_respond(c, 200, LWS_HTTP_HTML_TYPE, data, strlen(data));
        c->close_flag = 1;
    }
}

