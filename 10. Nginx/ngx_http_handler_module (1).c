


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


#include <arpa/inet.h>
#include <netinet/in.h>

typedef struct {
	int count;
	struct in_addr addr;
} ngx_pv_table;

ngx_pv_table pv_table[256] = {0}; //



void ngx_encode_http_page(char *html) {

	sprintf(html, "<h1>Flamingo, Apollo, liaoliao, bing</h1>");

	strcat(html, "<h2>");

	int i = 0;
	for (i = 0;i < 256;i ++) {

		if (pv_table[i].count != 0) {

			char str[INET_ADDRSTRLEN] = {0};
			char buffer[128] = {0};
			sprintf(buffer, "req from : %s, count: %d <br/>",
				inet_ntop(AF_INET, &pv_table[i].addr, str, sizeof(str)),
				pv_table[i].count);

			strcat(html, buffer);
		}
		
	}
	
	strcat(html, "</h2>");

} 

ngx_int_t ngx_http_count_handler(ngx_http_request_t *r) {

	// ip, count
	// 192.168.199.23
	struct sockaddr_in *cliaddr = (struct sockaddr_in*)r->connection->sockaddr;
	// search
	int idx = cliaddr->sin_addr.s_addr >> 24;

	pv_table[idx].count ++;
	memcpy(&pv_table[idx].addr, &cliaddr->sin_addr, sizeof(cliaddr->sin_addr));

	// encode page
	u_char html[1024] = {0};
	int len = sizeof(html);
	
	ngx_encode_http_page((char*)html);

	// send http response
	r->headers_out.status = 200;
	ngx_str_set(&r->headers_out.content_type, "text/html");
	ngx_http_send_header(r);


	ngx_buf_t *b = ngx_palloc(r->pool, sizeof(ngx_buf_t));
	b->pos = html;
	b->last = html + len;
	b->memory = 1;
	b->last_buf = 1;

	ngx_chain_t out;
	out.buf = b;
	out.next = NULL;
	
	return ngx_http_output_filter(r, &out);

}



char *ngx_http_handler_count_set(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {

	ngx_http_core_loc_conf_t *ccf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
	ccf->handler = ngx_http_count_handler;

	//memset(pv_table, 0, sizeof(pv_table));

	return NGX_CONF_OK;
}



ngx_command_t ngx_http_handler_module_cmd[] = {
	{
		ngx_string("count"),
		NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS,
		ngx_http_handler_count_set,
		NGX_HTTP_LOC_CONF_OFFSET,
		0,
		NULL,
	},
	ngx_null_command
};


// nginx -c conf/nginx.conf
// module 

static ngx_http_module_t ngx_http_handler_module_ctx = {

	NULL,
	NULL,

	NULL,
	NULL,

	NULL,
	NULL,

	NULL,
	NULL,

};


ngx_module_t ngx_http_handler_module = {
	NGX_MODULE_V1,
	&ngx_http_handler_module_ctx,
	ngx_http_handler_module_cmd,
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



