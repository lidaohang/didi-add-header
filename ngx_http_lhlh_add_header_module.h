#ifndef __NGX_HTTP_LHLH_ADD_HEADER_MODULE_H__
#define __NGX_HTTP_LHLH_ADD_HEADER_MODULE_H__


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#define LHLH_ADD_HEADER_HEADER_HINT                                "lhlh-header-hint-content"

#define LHLH_ADD_HEADER_TYPE_HEADER                                "header"
#define LHLH_ADD_HEADER_TYPE_PARAM                                 "param"
#define LHLH_ADD_HEADER_TYPE_VARIABLE                              "variable"

typedef struct {
    ngx_str_t       hint;
    ngx_str_t       header_value;
} ngx_http_lhlh_add_header_ctx_t;


typedef struct {
    ngx_flag_t      enable;
    ngx_str_t       header_type;
    ngx_str_t       header_name;
    ngx_str_t       header_rename;
} ngx_http_lhlh_add_header_conf_t;


#endif /* __NGX_HTTP_LHLH_ADD_HEADER_MODULE_H__ */
