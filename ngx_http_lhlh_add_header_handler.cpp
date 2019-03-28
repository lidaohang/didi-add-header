/*
 * Author: lidaohang 
 *
 * File: ngx_http_lhlh_add_header_handler.cpp
 * Create Date: 2016-10-13 12:03:15
 *
 */
#include "ngx_http_lhlh_add_header_handler.h"

static void
ngx_http_lhlh_add_header_write_json_value(yajl_gen &g, yajl_val &value)
{
    if (YAJL_IS_STRING(value)) {
        const char *s = YAJL_GET_STRING(value);
        yajl_gen_string(g, (unsigned char *)s, strlen(s));
    }
    else if (YAJL_IS_INTEGER(value)) {
        yajl_gen_integer(g, YAJL_GET_INTEGER(value));
    }
    else if (YAJL_IS_DOUBLE(value)) {
        yajl_gen_double(g, YAJL_GET_DOUBLE(value));
    }
    else if (YAJL_IS_TRUE(value)) {
        yajl_gen_bool(g, 1);
    }
    else if (YAJL_IS_FALSE(value)) {
        yajl_gen_bool(g, 0);
    }
    else if (YAJL_IS_NULL(value)) {
        yajl_gen_null(g);
    }
    else if (YAJL_IS_OBJECT(value)) {
        yajl_gen_map_open(g);

        int len = YAJL_GET_OBJECT(value)->len;
        for (int i = 0; i < len; i++) {
            const char *k = YAJL_GET_OBJECT(value)->keys[i];
            yajl_gen_string(g, (unsigned char *)k, strlen(k));
            ngx_http_lhlh_add_header_write_json_value(g, YAJL_GET_OBJECT(value)->values[i]);
        }

        yajl_gen_map_close(g);
    }
    else if (YAJL_IS_ARRAY(value)) {
        yajl_gen_array_open(g);

        int len = YAJL_GET_ARRAY(value)->len;
        for (int i = 0; i < len; i++) {
            ngx_http_lhlh_add_header_write_json_value(g, YAJL_GET_ARRAY(value)->values[i]);
        }

        yajl_gen_array_close(g);
    }
}


ngx_int_t
ngx_http_lhlh_add_header_request(ngx_http_request_t *r)
{
    size_t                                      i;
    size_t                                      len;
    yajl_gen                                    g;
    yajl_val                                    node;
    const unsigned char                         *buf;
    ngx_http_lhlh_add_header_ctx_t              *ctx;
    ngx_http_lhlh_add_header_conf_t             *lrcf;


    lrcf = (ngx_http_lhlh_add_header_conf_t *)ngx_http_get_module_loc_conf(r, ngx_http_lhlh_add_header_module);
    if ( lrcf == NULL ) {
        return NGX_ERROR;
    }

    ctx = (ngx_http_lhlh_add_header_ctx_t*)ngx_http_get_module_ctx(r, ngx_http_lhlh_add_header_module);
    if ( ctx == NULL ) {
        return NGX_ERROR;
    }

    if ( ctx->header_value.len == 0 || ctx->header_value.data == NULL ) {
        return NGX_ERROR;
    }

    g = yajl_gen_alloc(NULL);
    if ( g == NULL ) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
            "yajl_gen_alloc is null");
        return NGX_ERROR;
    }
    yajl_gen_config(g, yajl_gen_beautify, 0);

    yajl_gen_map_open(g);

    if ( ctx->hint.len > 0 && ctx->hint.data != NULL ) {
        //read json
        node = yajl_tree_parse((const char*)ctx->hint.data, NULL, 0);
        if ( node == NULL || (!YAJL_IS_OBJECT(node))) {
            yajl_gen_free(g);

            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                "yajl_tree_parse is null");

            return NGX_ERROR;
        }

        len = node->u.object.len;
        for ( i = 0; i < len; ++i ) {

            const char *k = node->u.object.keys[i];
            uint32_t k_len = strlen(k);

            if ( k_len == lrcf->header_name.len &&
                    ngx_strncasecmp((u_char*)k,lrcf->header_name.data, k_len) == 0) {
                    continue;
            }

            yajl_gen_string(g, (const unsigned char *)k, k_len);
            ngx_http_lhlh_add_header_write_json_value(g, node->u.object.values[i]);
        }
        yajl_tree_free(node);

        ctx->hint.data = NULL;
        ctx->hint.len = 0;
    }

    if ( lrcf->header_rename.len != 0 && lrcf->header_rename.data != NULL) {
        yajl_gen_string(g, (const unsigned char *) lrcf->header_rename.data, lrcf->header_rename.len);
    }else {
        yajl_gen_string(g, (const unsigned char *) lrcf->header_name.data, lrcf->header_name.len);
    }
    yajl_gen_string(g, ctx->header_value.data, ctx->header_value.len);

    yajl_gen_map_close(g);

    yajl_gen_status status = yajl_gen_get_buf(g, &buf, &len);
    if(status != yajl_gen_status_ok) {
        yajl_gen_free(g);

        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
            "yajl_gen_get_buf is null");

        return NGX_ERROR;
    }

    ctx->hint.len = len + 1;
    ctx->hint.data = (u_char*)ngx_pcalloc(r->pool, ctx->hint.len);
    if ( ctx->hint.data == NULL ) {
        yajl_gen_free(g);

        return NGX_ERROR;
    }
    ngx_memcpy(ctx->hint.data, buf, ctx->hint.len);

    yajl_gen_free(g);

    return NGX_OK;
}

