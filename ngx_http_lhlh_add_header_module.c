/* Author: lidaohang
 *
 * File: ngx_http_lhlh_add_header_module.c
 * Create Date: 2017-02-13 15:32:31
 *
 */
#include <ndk.h>
#include "ngx_http_lhlh_add_header_handler.h"

static void
ngx_http_lhlh_add_header_get_header(ngx_http_request_t *r);

static ngx_int_t
ngx_http_lhlh_add_header_get_param(ngx_http_request_t *r);

static ngx_int_t
ngx_http_lhlh_add_header_get_variable(ngx_http_request_t *r, ngx_str_t *value);

static ngx_int_t
ngx_http_lhlh_add_header_handler(ngx_http_request_t *r);

static char*
ngx_http_lhlh_add_header_name(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static char*
ngx_http_lhlh_add_header_type(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_int_t
ngx_http_lhlh_add_header_init(ngx_conf_t *cf);

static void *
ngx_http_lhlh_add_header_create_loc_conf(ngx_conf_t *cf);

static char*
ngx_http_lhlh_add_header_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);

static ngx_command_t  ngx_http_lhlh_add_header_commands[] = {

    { ngx_string("lhlh_add_header"),
        NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_FLAG,
        ngx_conf_set_flag_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_lhlh_add_header_conf_t, enable),
        NULL },

    { ngx_string("lhlh_add_header_type"),
        NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
        ngx_http_lhlh_add_header_type,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_lhlh_add_header_conf_t, header_type),
        NULL },

    { ngx_string("lhlh_add_header_name"),
        NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_SIF_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF|NGX_CONF_TAKE12,
        ngx_http_lhlh_add_header_name,
        NGX_HTTP_LOC_CONF_OFFSET,
        0,
        NULL },

        ngx_null_command
};


static ngx_http_module_t  ngx_http_lhlh_add_header_module_ctx = {
    NULL,                                         /* preconfiguration */
    ngx_http_lhlh_add_header_init,                /* postconfiguration */

    NULL,                                         /* create main configuration */
    NULL,                                         /* init main configuration */

    NULL,                                         /* create server configuration */
    NULL,                                         /* merge server configuration */

    ngx_http_lhlh_add_header_create_loc_conf,     /* create location configration */
    ngx_http_lhlh_add_header_merge_loc_conf       /* merge location configration */
};


ngx_module_t  ngx_http_lhlh_add_header_module = {
    NGX_MODULE_V1,
    &ngx_http_lhlh_add_header_module_ctx,         /* module context */
    ngx_http_lhlh_add_header_commands,            /* module directives */
    NGX_HTTP_MODULE,                              /* module type */
    NULL,                                         /* init master */
    NULL,                                         /* init module */
    NULL,                                         /* init process */
    NULL,                                         /* init thread */
    NULL,                                         /* exit thread */
    NULL,                                         /* exit process */
    NULL,                                         /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_int_t
ngx_http_lhlh_add_header_set_header(ngx_http_request_t *r)
{
    uint32_t                                len;
    u_char                                  *data;
    ngx_table_elt_t                         *header;
    ngx_list_part_t                         *part;
    ngx_uint_t                              i;
    ngx_http_lhlh_add_header_ctx_t          *ctx;


    ctx = ngx_http_get_module_ctx(r, ngx_http_lhlh_add_header_module);
    if ( ctx == NULL ) {
        return NGX_ERROR;
    }

    if ( ctx->hint.len == 0 || ctx->hint.data == NULL ) {
        return NGX_OK;
    }

    len = strlen(LHLH_ADD_HEADER_HEADER_HINT);
    data = (u_char*)LHLH_ADD_HEADER_HEADER_HINT;

    part = &r->headers_in.headers.part;
    header = part->elts;

    for (i = 0; /* void */; i++) {

      if (i >= part->nelts) {
        if (part->next == NULL) {
          break;
        }

        part = part->next;
        header = part->elts;
        i = 0;
      }

      if (header[i].key.len == len &&
          ngx_strncasecmp(header[i].key.data, data, len) == 0) {

        header[i].value.len = ctx->hint.len;
        header[i].value.data = ngx_pcalloc(r->pool, ctx->hint.len);
        if ( header[i].value.data == NULL ) {
          return NGX_ERROR;
        }
        ngx_memcpy(header[i].value.data, ctx->hint.data, ctx->hint.len);

        return NGX_OK;
      }
    }

    if (r->http_version < NGX_HTTP_VERSION_10) {
        return NGX_OK;
    }

    if ( r->headers_in.headers.size == 0 ) {
        return NGX_OK;
    }

    header = ngx_list_push(&r->headers_in.headers);
    if ( header == NULL ) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    header->hash = r->header_hash;

    header->key.len = len;
    header->key.data =  data;

    header->value.len = ctx->hint.len;
    header->value.data = ngx_pcalloc(r->pool, ctx->hint.len);
    if ( header->value.data == NULL ) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    ngx_memcpy(header->value.data, ctx->hint.data, ctx->hint.len);

    header->lowcase_key = ngx_pnalloc(r->pool, header->key.len);
    if (header->lowcase_key == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    ngx_strlow(header->lowcase_key, header->key.data, header->key.len);

    return NGX_OK;
}


static void
ngx_http_lhlh_add_header_get_header(ngx_http_request_t *r)
{
    uint32_t                                len, hint_len;
    u_char                                  *data, *hint_data;
    ngx_flag_t                              flag = 0;
    ngx_uint_t                              i;
    ngx_table_elt_t                         *header;
    ngx_list_part_t                         *part;
    ngx_http_lhlh_add_header_ctx_t          *ctx;
    ngx_http_lhlh_add_header_conf_t         *lrcf;


    lrcf = ngx_http_get_module_loc_conf(r, ngx_http_lhlh_add_header_module);
    if ( lrcf == NULL  ) {
        return;
    }

    ctx = ngx_http_get_module_ctx(r, ngx_http_lhlh_add_header_module);
    if ( ctx == NULL ) {
        return;
    }

    len = strlen(LHLH_ADD_HEADER_TYPE_HEADER);
    data = (u_char*)LHLH_ADD_HEADER_TYPE_HEADER;

    hint_len = strlen(LHLH_ADD_HEADER_HEADER_HINT);
    hint_data = (u_char*)LHLH_ADD_HEADER_HEADER_HINT;

    if (lrcf->header_type.len == len &&
              ngx_strncasecmp(lrcf->header_type.data, data,len) == 0 ) {
        flag = 1;
    }

    part = &r->headers_in.headers.part;
    header = part->elts;

    for (i = 0; /* void */; i++) {

        if (i >= part->nelts) {
            if (part->next == NULL) {
                break;
            }

            part = part->next;
            header = part->elts;
            i = 0;
        }

        if (header[i].key.len == hint_len &&
                ngx_strncasecmp(header[i].key.data, hint_data, hint_len) == 0) {

            ctx->hint.len = header[i].value.len;
            ctx->hint.data = header[i].value.data;

            continue;
        }

        if (flag && header[i].key.len == lrcf->header_name.len &&
                ngx_strncasecmp(header[i].key.data, lrcf->header_name.data, lrcf->header_name.len) == 0) {

            ctx->header_value.len = header[i].value.len;
            ctx->header_value.data = header[i].value.data;

            continue;
        }
    }
}


static ngx_int_t
ngx_http_lhlh_add_header_get_param(ngx_http_request_t *r)
{
    uint32_t                                    len;
    u_char                                      *data;
    ngx_http_lhlh_add_header_ctx_t              *ctx;
    ngx_http_lhlh_add_header_conf_t             *lrcf;


    lrcf = ngx_http_get_module_loc_conf(r, ngx_http_lhlh_add_header_module);
    if ( lrcf == NULL  ) {
        return NGX_ERROR;
    }

    ctx = ngx_http_get_module_ctx(r, ngx_http_lhlh_add_header_module);
    if ( ctx == NULL ) {
        return NGX_ERROR;
    }

    len = strlen(LHLH_ADD_HEADER_TYPE_PARAM);
    data = (u_char*)LHLH_ADD_HEADER_TYPE_PARAM;

    if (lrcf->header_type.len == len &&
              ngx_strncasecmp(lrcf->header_type.data, data,len) == 0 ) {

        ngx_http_arg(r, lrcf->header_name.data, lrcf->header_name.len, &ctx->header_value);
    }

    return NGX_OK;
}


static ngx_int_t
ngx_http_lhlh_add_header_get_variable(ngx_http_request_t *r, ngx_str_t *value)
{
    uint32_t                                    len;
    u_char                                      *data;
    ngx_http_lhlh_add_header_ctx_t              *ctx;
    ngx_http_lhlh_add_header_conf_t             *lrcf;


    lrcf = ngx_http_get_module_loc_conf(r, ngx_http_lhlh_add_header_module);
    if ( lrcf == NULL  ) {
        return NGX_ERROR;
    }

    ctx = ngx_http_get_module_ctx(r, ngx_http_lhlh_add_header_module);
    if ( ctx == NULL ) {
        return NGX_ERROR;
    }

    len = strlen(LHLH_ADD_HEADER_TYPE_VARIABLE);
    data = (u_char*)LHLH_ADD_HEADER_TYPE_VARIABLE;

    if (lrcf->header_type.len == len &&
              ngx_strncasecmp(lrcf->header_type.data, data,len) == 0 ) {

        ctx->header_value.len = value->len;
        ctx->header_value.data = ngx_pnalloc(r->pool, ctx->header_value.len);
        if ( ctx->header_value.data == NULL ) {
            return NGX_ERROR;
        }
        ngx_memcpy(ctx->header_value.data, value->data, ctx->header_value.len);
    }

    return NGX_OK;
}


static ngx_int_t
ngx_http_lhlh_add_header_handler(ngx_http_request_t *r)
{
    ngx_int_t                               rc;
    ngx_http_lhlh_add_header_ctx_t          *ctx;
    ngx_http_lhlh_add_header_conf_t         *lrcf;


    lrcf = ngx_http_get_module_loc_conf(r, ngx_http_lhlh_add_header_module);
    if ( lrcf == NULL ) {
        return NGX_DECLINED;
    }

    ctx = ngx_http_get_module_ctx(r, ngx_http_lhlh_add_header_module);
    if ( ctx == NULL ) {
        ctx = ngx_pcalloc(r->pool, sizeof(ngx_http_lhlh_add_header_ctx_t));
        if ( ctx == NULL ) {
            return NGX_ERROR;
        }

        ngx_http_set_ctx(r, ctx, ngx_http_lhlh_add_header_module);
    }

    if ( lrcf->enable == NGX_CONF_UNSET || lrcf->enable == 0 ) {
        return NGX_DECLINED;
    }

    ngx_http_lhlh_add_header_get_header(r);

    rc = ngx_http_lhlh_add_header_get_param(r);
    if ( rc != NGX_OK ) {
         return NGX_DECLINED;
    }

    rc = ngx_http_lhlh_add_header_request(r);
    if ( rc != NGX_OK ) {
          return NGX_DECLINED;
    }

    rc = ngx_http_lhlh_add_header_set_header(r);
    if ( rc != NGX_OK ) {
          return NGX_DECLINED;
    }

    return NGX_DECLINED;
}


static ngx_int_t
ngx_http_lhlh_add_header_set_if_empty(ngx_http_request_t *r, ngx_str_t *res,
    ngx_http_variable_value_t *v)
{
    ngx_int_t                           rc;
    ngx_http_variable_value_t           *cur_v;

    cur_v = &v[0];

    res->data = cur_v->data;
    res->len = cur_v->len;

    if ( res->len == 0 || res->data == NULL ) {
        return NGX_OK;
    }

    rc = ngx_http_lhlh_add_header_get_variable(r, res);
    if ( rc != NGX_OK ) {
         return NGX_OK;
    }

    rc = ngx_http_lhlh_add_header_request(r);
    if ( rc != NGX_OK ) {
         return NGX_OK;
    }

    rc = ngx_http_lhlh_add_header_set_header(r);
    if ( rc != NGX_OK ) {
          return NGX_OK;
    }

    return NGX_OK;
}


static char*
ngx_http_lhlh_add_header_name(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_str_t                                               s;
    ndk_set_var_t                                           filter;
    ngx_uint_t                                              len;
    ngx_str_t                                               *value;
    ngx_http_lhlh_add_header_conf_t                         *lrcf = conf;

    len = cf->args->nelts;
    value = cf->args->elts;

    if ( len == 1 ) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                "\"%V\" must have \"didi_add_header_name\" parameter",
                &cmd->name);
        return NGX_CONF_ERROR;
    }

    if ( len == 3 ) {
        lrcf->header_rename.len = value[2].len;
        lrcf->header_rename.data = ngx_pcalloc(cf->pool, lrcf->header_rename.len);
        if ( lrcf->header_rename.data == NULL ) {
            return NGX_CONF_ERROR;
        }
        ngx_memcpy(lrcf->header_rename.data, value[2].data, lrcf->header_rename.len);
    }

    lrcf->header_name.len = value[1].len;
    lrcf->header_name.data = ngx_pcalloc(cf->pool, lrcf->header_name.len);
    if ( lrcf->header_name.data == NULL ) {
        return NGX_CONF_ERROR;
    }
    ngx_memcpy(lrcf->header_name.data, value[1].data, lrcf->header_name.len);

    if ( lrcf->header_name.len == 0 || lrcf->header_name.data == NULL ) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
            "\"%V\" must have \"didi_add_header_name\" parameter",
            &cmd->name);
        return NGX_CONF_ERROR;
    }

    s.len = lrcf->header_name.len + 1;
    s.data = ngx_pcalloc(cf->pool, s.len);
    if ( s.data == NULL ) {
        return NGX_CONF_ERROR;
    }
    ngx_sprintf(s.data, "$%V", &lrcf->header_name);

    filter.type = NDK_SET_VAR_MULTI_VALUE;
    filter.func = (void *) ngx_http_didi_add_header_set_if_empty;
    filter.size = 1;
    filter.data = NULL;

    return  ndk_set_var_multi_value_core(cf, &s, &s, &filter);
}


static char*
ngx_http_lhlh_add_header_type(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_didi_add_header_conf_t                         *lrcf = conf;

    ngx_conf_set_str_slot(cf, cmd, conf);

    if ( lrcf->header_type.len == 0 || lrcf->header_type.data == NULL ) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
            "\"%V\" must have \"didi_add_header_type\" parameter",
            &cmd->name);

        return NGX_CONF_ERROR;
    }

    if (lrcf->header_type.len == strlen(LHLH_ADD_HEADER_TYPE_HEADER) &&
              ngx_strncasecmp(lrcf->header_type.data, (u_char*)LHLH_ADD_HEADER_TYPE_HEADER,
              strlen(LHLH_ADD_HEADER_TYPE_HEADER)) == 0 ) {

    }
    else if (lrcf->header_type.len == strlen(LHLH_ADD_HEADER_TYPE_PARAM) &&
              ngx_strncasecmp(lrcf->header_type.data, (u_char*)LHLH_ADD_HEADER_TYPE_PARAM,
              strlen(LHLH_ADD_HEADER_TYPE_PARAM)) == 0 ) {

    }
    else if (lrcf->header_type.len == strlen(LHLH_ADD_HEADER_TYPE_VARIABLE) &&
              ngx_strncasecmp(lrcf->header_type.data, (u_char*)LHLH_ADD_HEADER_TYPE_VARIABLE,
              strlen(LHLH_ADD_HEADER_TYPE_VARIABLE)) == 0 ) {

    }
    else {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
            "lhlh_add_header_type  parameter header|param|variable",
            &cmd->name);

        return NGX_CONF_ERROR;
    }

    return NGX_CONF_OK;
}


static void *
ngx_http_lhlh_add_header_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_lhlh_add_header_conf_t      *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_lhlh_add_header_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->enable = NGX_CONF_UNSET;

    ngx_str_null(&conf->header_type);
    ngx_str_null(&conf->header_name);


    return conf;
}


static char*
ngx_http_lhlh_add_header_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_lhlh_add_header_conf_t      *prev = parent;
    ngx_http_lhlh_add_header_conf_t      *conf = child;

    ngx_conf_merge_value(conf->enable, prev->enable, 0);
    ngx_conf_merge_str_value(conf->header_type, prev->header_type, "");
    ngx_conf_merge_str_value(conf->header_name, prev->header_name, "");

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_lhlh_add_header_init(ngx_conf_t *cf)
{
    ngx_http_handler_pt                     *h;
    ngx_http_core_main_conf_t               *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    h = ngx_array_push(&cmcf->phases[NGX_HTTP_REWRITE_PHASE].handlers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    *h = ngx_http_lhlh_add_header_handler;

    return NGX_OK;
}

