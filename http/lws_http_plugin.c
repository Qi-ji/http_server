#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "lws_log.h"
#include "lws_http.h"
#include "lws_http_plugin.h"
#include "lws_util.h"

int lws_default_handler(lws_http_conn_t *c, int ev, void *p)
{
    struct http_message *hm = p;
    char data[1024] = {0};

    if (hm && ev == LWS_EV_HTTP_REQUEST) {
        if (c->send == NULL) {
            return HTTP_INTERNAL_SERVER_ERROR;
        }

        sprintf(data, "%s",
                  "<html><body><h>Enjoy your webserver!</h><br/><br/>"
                  "<ul style=\"list-style-type:circle\">"
                  "<li><a href=\"/hello\"> echo hello message </a></li>"
                  "<li><a href=\"/version\"> echo lws version </a></li>"
                  "<li><a href=\"/show.jpg\"> show picture </a></li>"
                  "<li><a href=\"/binary.tgz\"> downlad file </a></li>"
                  "</ul>"
                  "</body></html>");

        lws_http_respond(c, 200, c->close_flag, LWS_HTTP_HTML_TYPE, data, strlen(data));
    } else {
        return HTTP_BAD_REQUEST;
    }

    return HTTP_OK;
}

int lws_hello_handler(lws_http_conn_t *c, int ev, void *p)
{
    struct http_message *hm = p;
    char data[1024] = {0};

    if (hm && ev == LWS_EV_HTTP_REQUEST) {
        if (c->send == NULL) {
            return HTTP_INTERNAL_SERVER_ERROR;
        }

        sprintf(data, "%s",
                  "<html><body><h>Hello LWS!</h><br/><br/>"
                  "</body></html>");

        lws_http_respond(c, 200, c->close_flag, LWS_HTTP_HTML_TYPE, data, strlen(data));
    } else {
        return HTTP_BAD_REQUEST;
    }

    return HTTP_OK;
}

int lws_version_handler(lws_http_conn_t *c, int ev, void *p)
{
    struct http_message *hm = p;
    char data[1024] = {0};

    if (hm && ev == LWS_EV_HTTP_REQUEST) {
        if (c->send == NULL) {
            return HTTP_INTERNAL_SERVER_ERROR;
        }

        sprintf(data, "<html><body><h>LWS - version[%s]</h><br/><br/>"
                  "</body></html>", LWS_HTTP_VERSION);

        lws_http_respond(c, 200, c->close_flag, LWS_HTTP_HTML_TYPE, data, strlen(data));
    } else {
        return HTTP_BAD_REQUEST;
    }

    return HTTP_OK;
}

int lws_show_handler(lws_http_conn_t *c, int ev, void *p)
{
    struct http_message *hm = p;
    char *data = NULL;
    char *filename = NULL;
    long filesize = 0;
    int rlen = 0;
    char uri[128] = {0};
    char path[128] = {0};

    if (hm && ev == LWS_EV_HTTP_REQUEST) {
        strncpy(uri, hm->uri.p, hm->uri.len);
        filename = lws_basename(uri);
        if (filename == NULL)
            return HTTP_INTERNAL_SERVER_ERROR;

        sprintf(path, "./load/%s", filename);
        lws_log(4, "path: %s\n", path);
        filesize = lws_ftell_file(path);
        if (filesize <= 0)
            return HTTP_INTERNAL_SERVER_ERROR;

        lws_log(4, "filesize: %d\n", filesize);
        data = malloc(filesize);
        rlen = lws_read_file(path, data, filesize);
        if (rlen != filesize) {
            lws_log(2, "rlen: %d\n", rlen);
            free(data);
            return HTTP_INTERNAL_SERVER_ERROR;
        }

        lws_http_respond(c, 200, c->close_flag, LWS_HTTP_JPEG_TYPE, data, rlen);
        free(data);
    } else {
        return HTTP_BAD_REQUEST;
    }

    return HTTP_OK;
}

int lws_binary_handler(lws_http_conn_t *c, int ev, void *p)
{
    struct http_message *hm = p;
    char *data = NULL;
    char *filename = NULL;
    long filesize = 0;
    int rlen = 0;
    char uri[128] = {0};
    char path[128] = {0};

    if (hm && ev == LWS_EV_HTTP_REQUEST) {
        strncpy(uri, hm->uri.p, hm->uri.len);
        filename = lws_basename(uri);
        if (filename == NULL)
            return HTTP_INTERNAL_SERVER_ERROR;

        sprintf(path, "./load/%s", filename);
        lws_log(4, "path: %s\n", path);
        filesize = lws_ftell_file(path);
        if (filesize <= 0)
            return HTTP_INTERNAL_SERVER_ERROR;

        lws_log(4, "filesize: %d\n", filesize);
        data = malloc(filesize);
        rlen = lws_read_file(path, data, filesize);
        if (rlen != filesize) {
            lws_log(2, "rlen: %d\n", rlen);
            free(data);
            return HTTP_INTERNAL_SERVER_ERROR;
        }

        lws_http_respond(c, 200, c->close_flag, LWS_HTTP_OCTET_STREAM, data, rlen);
        free(data);
    } else {
        return HTTP_BAD_REQUEST;
    }

    return HTTP_OK;
}

