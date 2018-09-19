#ifndef _LWS_UTIL_H_
#define _LWS_UTIL_H_

/**
 * @func    lws_basename
 * @brief   get base filename /home/cfg/settings.json -> settings.json
 *
 * @param   filename[in] input filename
 * @return  On success, return basename. Or return input filename.
 **/
extern char *lws_basename(char *filename);

/**
 * @func    lws_ftell_file
 * @brief   get file size
 *
 * @param   filename[in] input file name
 * @return  On success, return file size. On error, return -1.
 **/
extern long lws_ftell_file(char *filename);

/**
 * @func    lws_read_file
 * @brief   read file data
 *
 * @param   filename[in] input file name
 * @param   data[out] file data
 * @param   size[int] read length
 * @return  On success, return read size. On error, return -1.
 **/
extern int lws_read_file(char *filename, void *data, int size);

#endif // _LWS_UTIL_H_

