#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

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
                  "<li><a href=\"/download\"> downlad file </a></li>"
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

int lws_download_handler(lws_http_conn_t *c, int ev, void *p)
{
    struct http_message *hm = p;
    char uri[1024] = {0};
    char path[1024] = {0};
    char *filename;
    struct stat s_buf;
    char *data = NULL;
    long filesize = 0;
    int rlen = 0;
    DIR *dp = NULL;
    struct dirent *dir;

    if (hm == NULL || ev != LWS_EV_HTTP_REQUEST)
        return HTTP_BAD_REQUEST;

    if (hm->uri.len > 1024) {
        lws_log(2, "Error, request size: %d\n", hm->uri.len);
        return HTTP_BAD_REQUEST;
    }

    lws_log(4, "%.*s\n", hm->uri.len, hm->uri.p);
    strncpy(uri, hm->uri.p, hm->uri.len);
    filename = uri + strlen("/download");
    if (filename == NULL)
        sprintf(path, "./load");
    else {
        sprintf(path, "./load%s", filename);
    }

    if (access(path, F_OK) != 0) {
        lws_log(2, "path[%s] is not exist\n", path);
        return HTTP_NOT_FOUND;
    }

    stat(path, &s_buf);
    if (S_ISDIR(s_buf.st_mode)) {
        lws_log(4, "show dir: %s\n", path);

        data = malloc(4 * 1024);
        rlen += sprintf(data + rlen, "<html><head><title>%s</title></head><body>", path);
        rlen += sprintf(data + rlen, "<h1>Index of %s</h1>", path);
        dp = opendir(path);
        while ((dir = readdir(dp)) != NULL) {
            if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
                continue;

            if (dir->d_type == DT_DIR) {
                rlen += sprintf(data + rlen, "<a href=\"%s/%s\">./%s</a></br>", uri, dir->d_name, dir->d_name);
            } else {
                rlen += sprintf(data + rlen, "<a href=\"%s/%s\">%s</a></br>", uri, dir->d_name, dir->d_name);
            }
        }
        closedir(dp);
        rlen += sprintf(data + rlen, "</body></html>");
        lws_log(4, "response: %.*s\n", rlen, data);
        lws_http_respond(c, 200, c->close_flag, LWS_HTTP_HTML_TYPE, data, rlen);
        free(data);
    } else if (S_ISREG(s_buf.st_mode)) {
        lws_log(4, "show file: %s\n", path);
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

        lws_http_respond(c, 200, c->close_flag, lws_http_contenttype(path), data, rlen);
        free(data);
    }

    return HTTP_OK;
}

