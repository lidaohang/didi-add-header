#ifndef __NGX_HTTP_DIDI_ADD_HEADER_HANDLER_H__
#define __NGX_HTTP_DIDI_ADD_HEADER_HANDLER_H__

#if __cplusplus
extern "C" {
#endif

#include <yajl/yajl_parse.h>
#include <yajl/yajl_gen.h>
#include <yajl/yajl_tree.h>

#include "ngx_http_didi_add_header_module.h"

extern ngx_module_t ngx_http_didi_add_header_module;

ngx_int_t
ngx_http_didi_add_header_request(ngx_http_request_t *r);


#if __cplusplus
}
#endif



#endif /* __NGX_HTTP_DIDI_ADD_HEADER_HANDLER_H__ */
