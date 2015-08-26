/*
	Copyright (C) Paul 
	rwhsysu@163.com

	for nginx.conf 

	location /mycookie {
		cookieflag on;
	} 


*/

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


static ngx_int_t ngx_http_mycookie_init(ngx_conf_t *cf); //handler挂载函数,  content phase handlers

static void * ngx_http_mycookie_create_loc_conf(ngx_conf_t *cf);

static ngx_int_t ngx_http_mycookie_handler(ngx_http_request_t *r); //handler 函数定义


typedef struct 
{
	ngx_flag_t cookieflag;
}ngx_http_mycookie_loc_conf_t;


static ngx_command_t ngx_http_mycookie_commands[]={
	{
		ngx_string("cookieflag"),
		NGX_HTTP_LOC_CONF|NGX_CONF_FLAG,/*
		NGX_CONF_FLAG：配置指令可以接受的值是”on”或者”off”，最终会被转成bool值。
		*/
		ngx_conf_set_flag_slot,
		NGX_HTTP_LOC_CONF_OFFSET,
		offsetof(ngx_http_mycookie_loc_conf_t,cookieflag),
		NULL
	},
	ngx_null_command
};

static ngx_http_module_t ngx_http_mycookie_module_ctx = {
        NULL,                          /* preconfiguration */
        ngx_http_mycookie_init,           /* postconfiguration */

        NULL,                          /* create main configuration */
        NULL,                          /* init main configuration */

        NULL,                          /* create server configuration */
        NULL,                          /* merge server configuration */

        ngx_http_mycookie_create_loc_conf, /* create location configuration */
        NULL                            /* merge location configuration */
};

ngx_module_t ngx_http_mycookie_module = {
        NGX_MODULE_V1,
        &ngx_http_mycookie_module_ctx,    /* module context */
        ngx_http_mycookie_commands,       /* module directives */
        NGX_HTTP_MODULE,               /* module type */
        NULL,                          /* init master */
        NULL,                          /* init module */
        NULL,                          /* init process */
        NULL,                          /* init thread */
        NULL,                          /* exit thread */
        NULL,                          /* exit process */
        NULL,                          /* exit master */
        NGX_MODULE_V1_PADDING
};

static ngx_int_t ngx_http_mycookie_init(ngx_conf_t *cf)
{
	ngx_http_handler_pt  *h;
	ngx_http_core_main_conf_t *cmcf;
	cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module); //取得core_module的cf

        h = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers); // 挂载函数到对应处理阶段
        /*
        为了更精细地控制对于客户端请求的处理过程，nginx把这个处理过程划分成了11个阶段。
        他们从前到后，依次列举如下：
		NGX_HTTP_POST_READ_PHASE:
		 	读取请求内容阶段
		NGX_HTTP_SERVER_REWRITE_PHASE:
		 	Server请求地址重写阶段
		NGX_HTTP_FIND_CONFIG_PHASE:
		 	配置查找阶段:
		NGX_HTTP_REWRITE_PHASE:
		 	Location请求地址重写阶段
		NGX_HTTP_POST_REWRITE_PHASE:
		 	请求地址重写提交阶段
		NGX_HTTP_PREACCESS_PHASE:
		 	访问权限检查准备阶段
		NGX_HTTP_ACCESS_PHASE:
		 	访问权限检查阶段
		NGX_HTTP_POST_ACCESS_PHASE:
		 	访问权限检查提交阶段
		NGX_HTTP_TRY_FILES_PHASE:
		 	配置项try_files处理阶段
		NGX_HTTP_CONTENT_PHASE:
		 	内容产生阶段
		NGX_HTTP_LOG_PHASE:
		 	日志模块处理阶段
        */
        if (h == NULL) {
                return NGX_ERROR;
        }

        *h = ngx_http_mycookie_handler; //将函数指针指向handler函数

        return NGX_OK;
}

static void *ngx_http_mycookie_create_loc_conf(ngx_conf_t *cf)
{
        ngx_http_mycookie_loc_conf_t* local_conf = NULL;
        local_conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_mycookie_loc_conf_t));
        if (local_conf == NULL)
        {
                return NULL;
        }

        //ngx_str_null(&local_conf->teststr);
        /*
		conf->teststr.len = 0;
        conf->teststr.data = NULL;
        */
        local_conf->cookieflag = NGX_CONF_UNSET;
        //local_conf->testnum = NGX_CONF_UNSET;
        //local_conf->testsize = NGX_CONF_UNSET_SIZE;
        /*
		这些配置信息一般默认都应该设为一个未初始化的值，针对这个需求，Nginx定义了一系列的宏定义来代表各种配置所对应数据类型的未初始化值，如下：
		#define NGX_CONF_UNSET       -1
		#define NGX_CONF_UNSET_UINT  (ngx_uint_t) -1
		#define NGX_CONF_UNSET_PTR   (void *) -1
		#define NGX_CONF_UNSET_SIZE  (size_t) -1
		#define NGX_CONF_UNSET_MSEC  (ngx_msec_t) -1
        */

        return local_conf;
}

static ngx_int_t ngx_http_mycookie_handler(ngx_http_request_t *r)
{
	ngx_int_t rc;
	ngx_buf_t *b;
	ngx_chain_t out;
	ngx_http_mycookie_loc_conf_t *my_cf;
	u_char ngx_my_string[1024] = {0};
	ngx_uint_t content_length = 0;
	

	ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "ngx_http_mycookie_handler is called!");

	my_cf = ngx_http_get_module_loc_conf(r,ngx_http_mycookie_module);
	if (my_cf->cookieflag == NGX_CONF_UNSET )
        {
                ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "cookieflag is UNSET!");
                return NGX_DECLINED;
        }
    //从请求头读取cookies，使用的时候可以使用cookies[0]->value.data,cookies[0]->value.len
    ngx_table_elt_t ** cookies = NULL;
    cookies = r->headers_in.cookies.elts; 

    ngx_table_elt_t * client_ip = NULL;
    client_ip = r->headers_in.user_agent;
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
	 //u_char ipchar[client_ip->value.len+1]={0};
	 //u_char cookiechar[cookies[0]->value.len+1]={0};
	 //ngx_sprintf(ipchar, "%V\0",&client_ip->value);
	 //ngx_sprintf(cookiechar, "%V\0",&cookies[0]->value);

    if ( cookies != NULL)  //未定义cookie？
        {
                ngx_sprintf(ngx_my_string, "user_agent is %V, you have cookie", 
                	&client_ip->value);
        }
        else
        {
                ngx_sprintf(ngx_my_string, "user_agent is %V, you do not have cookie or cookie not match", 
                	&client_ip->value);
                //没有cookie，则给他cookie
                ngx_table_elt_t  *set_cookie = ngx_list_push(&r->headers_out.headers);
			    if (set_cookie == NULL) {                          
			        return NGX_ERROR;                              
			    }                                                  
			 
			    set_cookie->hash = 1;
			    set_cookie->key.len = sizeof("Set-Cookie") - 1;
			    set_cookie->key.data = (u_char *) "Set-Cookie";
			    set_cookie->value = client_ip->value;
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
