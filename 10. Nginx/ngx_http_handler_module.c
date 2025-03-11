


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>



typedef struct {
	ngx_flag_t enable;
} ngx_http_filter_conf_t;

/*
header
 */

static ngx_str_t prefix = ngx_string("<h2>King </h2>");

static ngx_http_output_header_filter_pt  ngx_http_next_header_filter;
static ngx_http_output_body_filter_pt    ngx_http_next_body_filter;


ngx_int_t ngx_http_zry_header_filter(ngx_http_request_t *r) {


	r->headers_out.content_length_n += prefix.len;

	return ngx_http_next_header_filter(r);
}

ngx_int_t ngx_http_zry_body_filter(ngx_http_request_t *r, ngx_chain_t *chain) {

	ngx_buf_t *b = ngx_create_temp_buf(r->pool, prefix.len);
	b->start = b->pos = prefix.data;
	b->last = b->pos + prefix.len;

	ngx_chain_t *c1 = ngx_alloc_chain_link(r->pool);
	c1->buf = b;
	c1->next = chain;

	return ngx_http_next_body_filter(r, c1);

}



ngx_int_t   ngx_http_zry_filter_init(ngx_conf_t *cf) {

	ngx_http_next_header_filter = ngx_http_top_header_filter;
	ngx_http_top_header_filter = ngx_http_zry_header_filter;

	ngx_http_next_body_filter = ngx_http_top_body_filter;
	ngx_http_top_body_filter = ngx_http_zry_body_filter;

	return NGX_OK;
}


void *ngx_http_zry_filter_create_loc_conf(ngx_conf_t *cf) {

	ngx_http_filter_conf_t *conf = ngx_palloc(cf->pool, sizeof(ngx_http_filter_conf_t));
	if (conf == NULL) return NULL;

	conf->enable = NGX_CONF_UNSET;

	return conf;
}


char *ngx_http_zry_filter_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child) {

	ngx_http_filter_conf_t *prev = (ngx_http_filter_conf_t*)parent;
	ngx_http_filter_conf_t *next = (ngx_http_filter_conf_t*)child;

	//printf("ngx_http_zry_filter_merge_loc_conf: %d\n", next->enable);
	ngx_conf_merge_value(next->enable, prev->enable, 0);

	return NGX_CONF_OK;

}

/*
char *ngx_http_zry_filter_set_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {


	char *p = conf;

	ngx_flag_t *flag = (p + cmd->offset);
	if (*flag == NGX_CONF_UNSET) {
		
	}
	
	

	return NGX_CONF_OK;

}
*/

/*
struct ngx_command_s {
    ngx_str_t             name;
    ngx_uint_t            type;
    char               *(*set)(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
    ngx_uint_t            conf;
    ngx_uint_t            offset;
    void                 *post;
};

*/

// prefix on/off


ngx_command_t ngx_http_zry_filter_module_cmd[] = {
	{
		ngx_string("prefix"),
		NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_CONF_FLAG,
		//ngx_http_zry_filter_set_slot,
		ngx_conf_set_flag_slot,
		NGX_HTTP_LOC_CONF_OFFSET,
		offsetof(ngx_http_filter_conf_t, enable),
		NULL,
	},
	ngx_null_command
};


// nginx -c conf/nginx.conf
// module 

static ngx_http_module_t ngx_http_zry_filter_module_ctx = {

	NULL,
	ngx_http_zry_filter_init,

	NULL,
	NULL,

	NULL,
	NULL,

	ngx_http_zry_filter_create_loc_conf,
	ngx_http_zry_filter_merge_loc_conf,

};


ngx_module_t ngx_http_zry_filter_module = {
	NGX_MODULE_V1,
	&ngx_http_zry_filter_module_ctx,
	ngx_http_zry_filter_module_cmd,
	NGX_HTTP_MODULE,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NGX_MODULE_V1_PADDING
};



