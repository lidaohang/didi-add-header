#ifndef __NGX_HTTP_DIDI_ADD_HEADER_MODULE_H__
#define __NGX_HTTP_DIDI_ADD_HEADER_MODULE_H__


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#define DIDI_ADD_HEADER_HEADER_HINT                                "didi-header-hint-content"

#define DIDI_ADD_HEADER_TYPE_HEADER                                "header"
#define DIDI_ADD_HEADER_TYPE_PARAM                                 "param"
#define DIDI_ADD_HEADER_TYPE_VARIABLE                              "variable"

typedef struct {
    ngx_str_t       hint;
    ngx_str_t       header_value;
} ngx_http_didi_add_header_ctx_t;


typedef struct {
    ngx_flag_t      enable;
    ngx_str_t       header_type;
    ngx_str_t       header_name;
    ngx_str_t       header_rename;
} ngx_http_didi_add_header_conf_t;


#endif /* __NGX_HTTP_DIDI_ADD_HEADER_MODULE_H__ */
