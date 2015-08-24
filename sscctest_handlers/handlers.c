
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_http_sscctest_module.h>

ngx_int_t ngx_http_ssscctest_realhandler_test(ngx_http_sscctest_request_t *req, ngx_http_sscctest_response_t *resp)
{
	ngx_int_t rc;
	ngx_buf_t *b;
	ngx_chain_t out;
	ngx_http_configprint_loc_conf_t *my_cf;
	u_char ngx_my_string[1024] = {0};
	ngx_uint_t content_length = 0;
	

	ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "ngx_http_configprint_handler is called!");

	my_cf = ngx_http_get_module_loc_conf(r,ngx_http_configprint_module);
	if (my_cf->teststr.len == 0 )
        {
                ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "printstr is empty!");
                return NGX_DECLINED;
        }

    /*
	 * supported formats:
	 *    %[0][width][x][X]O        off_t
	 *    %[0][width]T              time_t
	 *    %[0][width][u][x|X]z      ssize_t/size_t
	 *    %[0][width][u][x|X]d      int/u_int
	 *    %[0][width][u][x|X]l      long
	 *    %[0][width|m][u][x|X]i    ngx_int_t/ngx_uint_t
	 *    %[0][width][u][x|X]D      int32_t/uint32_t
	 *    %[0][width][u][x|X]L      int64_t/uint64_t
	 *    %[0][width|m][u][x|X]A    ngx_atomic_int_t/ngx_atomic_uint_t
	 *    %[0][width][.width]f      double, max valid number fits to %18.15f
	 *    %P                        ngx_pid_t
	 *    %M                        ngx_msec_t
	 *    %r                        rlim_t
	 *    %p                        void *
	 *    %V                        ngx_str_t *
	 *    %v                        ngx_variable_value_t *
	 *    %s                        null-terminated string
	 *    %*s                       length and string
	 *    %Z                        '\0'
	 *    %N                        '\n'
	 *    %c                        char
	 *    %%                        %
	 *
	 *  reserved:
	 *    %t                        ptrdiff_t
	 *    %S                        null-terminated wchar string
	 *    %C                        wchar
	 */
	 ngx_table_elt_t * host = r->headers_in.host;
	 //ngx_connection_local_sockaddr(r->connection, NULL, 0);
	 struct sockaddr_in *ip = (struct sockaddr_in *) (r->connection->sockaddr); //inet_ntoa(ip->sin_addr)

    if (my_cf->testflag == NGX_CONF_UNSET
                || my_cf->testflag == 0)
        {
                ngx_sprintf(ngx_my_string, "<strong> <font color=\"red\"> printstr =</font> %V, <font color=\"red\">printnum =</font>  %i, <font color=\"red\">printsize =</font>  %z</strong>", 
                	&my_cf->teststr, my_cf->testnum, my_cf->testsize);
        }
        else
        {
                ngx_sprintf(ngx_my_string, "<strong> <font color=\"red\">printstr =</font>  %V, <font color=\"red\">printnum =</font>  %i, \
                	<font color=\"red\">printsize =</font>  %z, <font color=\"red\">Visited Times:</font> %d, \
                	<font color=\"red\">URI:</font> %V,  <font color=\"red\">request_line:</font> %V, <font color=\"red\">host:</font> %V\
                	, <font color=\"red\">ip:</font> %s :%D, <font color=\"red\">args:</font> %V</strong>",
                	&my_cf->teststr, my_cf->testnum, my_cf->testsize, ++ngx_configprint_visited_times, &r->uri, &r->request_line, &host->value, inet_ntoa(ip->sin_addr) , ntohs(ip->sin_port), &r->args); 
        }
    content_length = ngx_strlen(ngx_my_string);

    /* we response to 'GET' and 'HEAD' requests only */
        if (!(r->method & (NGX_HTTP_GET|NGX_HTTP_HEAD))) {
                return NGX_HTTP_NOT_ALLOWED;
        }

        /* discard request body, since we don't need it here */
        rc = ngx_http_discard_request_body(r);

        if (rc != NGX_OK) {
                return rc;
        }
		/* set the 'Content-type' header */
        /*
         *r->headers_out.content_type.len = sizeof("text/html") - 1;
         *r->headers_out.content_type.data = (u_char *)"text/html";
         */
        ngx_str_set(&r->headers_out.content_type, "text/html");
        r->headers_out.status = NGX_HTTP_OK;
        r->headers_out.content_length_n = content_length;
        /* send the header only, if the request type is http 'HEAD' */
        if (r->method == NGX_HTTP_HEAD) {
            	return ngx_http_send_header(r);
        }

        /* allocate a buffer for your response body */
        b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
        if (b == NULL) {
                return NGX_HTTP_INTERNAL_SERVER_ERROR;
        }

        /*另一种方式
        对于创建temporary字段为1的buf（就是其内容可以被后续的filter模块进行修改），可以直接使用函数ngx_create_temp_buf进行创建。

		ngx_buf_t *ngx_create_temp_buf(ngx_pool_t *pool, size_t size);
		该函数创建一个ngx_but_t类型的对象，并返回指向这个对象的指针，创建失败返回NULL。

		对于创建的这个对象，它的start和end指向新分配内存开始和结束的地方。pos和last都指向这块新分配内存的开始处，这样，后续的操作可以在这块新分配的内存上存入数据。*/
        
        /* adjust the pointers of the buffer */
        b->pos = ngx_my_string;
        b->last = ngx_my_string + content_length;
        b->memory = 1;    /* this buffer is in memory */
        b->last_buf = 1;  /* this is the last buffer in the buffer chain */

        /* attach this buffer to the buffer chain */
        out.buf = b;
        out.next = NULL;
        /* send the headers of your response */
        rc = ngx_http_send_header(r);

        if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
                return rc;
        }

        /* send the buffer chain of your response */
        return ngx_http_output_filter(r, &out);
}
