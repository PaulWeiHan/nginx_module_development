/*
	Copyright (C) Paul 
	rwhsysu@163.com

	for nginx.conf 

	location /configprint {
		printstr paul;
		printflag on;
		printnum 9;
		printsize 199;
	} 


*/

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


static ngx_int_t ngx_http_configprint_init(ngx_conf_t *cf); //handler挂载函数,  content phase handlers

static void * ngx_http_configprint_create_loc_conf(ngx_conf_t *cf);

static ngx_int_t ngx_http_configprint_handler(ngx_http_request_t *r); //handler 函数定义

static int ngx_configprint_visited_times = 0;

// static char*
// ngx_http_configprint_printstr(ngx_conf_t *cf, ngx_command_t *cmd, void *conf); //加载配置
// static char*
// ngx_http_configprint_printflag(ngx_conf_t *cf, ngx_command_t *cmd, void *conf); //加载配置
// static char*
// ngx_http_configprint_printnum(ngx_conf_t *cf, ngx_command_t *cmd, void *conf); //加载配置
// static char*
// ngx_http_configprint_printsize(ngx_conf_t *cf, ngx_command_t *cmd, void *conf); //加载配置



typedef struct 
{
	/* data */
	ngx_str_t teststr;
	ngx_flag_t testflag;
	ngx_int_t testnum;
	size_t testsize;

}ngx_http_configprint_loc_conf_t;

static ngx_command_t ngx_http_configprint_commands[]={
	{
		ngx_string("printstr"),
        NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS|NGX_CONF_TAKE1,/*
		NGX_HTTP_LOC_CONF: 可以出现在http server块里面的location配置指令里。
		NGX_CONF_NOARGS：配置指令不接受任何参数。
		or  NGX_CONF_TAKE1：配置指令接受1个参数。
        */
        ngx_conf_set_str_slot, /*配置处理函数

		nginx已经默认提供了对一些标准类型的参数进行读取的函数，可以直接赋值给set字段使用。下面来看一下这些已经实现的set类型函数。
		ngx_conf_set_flag_slot： 读取NGX_CONF_FLAG类型的参数。
		ngx_conf_set_str_slot:读取字符串类型的参数。
		ngx_conf_set_str_array_slot: 读取字符串数组类型的参数。
		ngx_conf_set_keyval_slot： 读取键值对类型的参数。
		ngx_conf_set_num_slot: 读取整数类型(有符号整数ngx_int_t)的参数。
		ngx_conf_set_size_slot:读取size_t类型的参数，也就是无符号数。
		ngx_conf_set_off_slot: 读取off_t类型的参数。
		ngx_conf_set_msec_slot: 读取毫秒值类型的参数。
		ngx_conf_set_sec_slot: 读取秒值类型的参数。
		ngx_conf_set_bufs_slot： 读取的参数值是2个，一个是buf的个数，一个是buf的大小。例如： output_buffers 1 128k;
		ngx_conf_set_enum_slot: 读取枚举类型的参数，将其转换成整数ngx_uint_t类型。
		ngx_conf_set_bitmask_slot: 读取参数的值，并将这些参数的值以bit位的形式存储。例如：HttpDavModule模块的dav_methods指令。
        */
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_configprint_loc_conf_t, teststr),
        NULL
	},
	{
		ngx_string("printflag"),
		NGX_HTTP_LOC_CONF|NGX_CONF_FLAG,/*
		NGX_CONF_FLAG：配置指令可以接受的值是”on”或者”off”，最终会被转成bool值。
		*/
		ngx_conf_set_flag_slot,
		NGX_HTTP_LOC_CONF_OFFSET,
		offsetof(ngx_http_configprint_loc_conf_t,testflag),
		NULL
	},
	{
		ngx_string("printnum"),
		NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,/*
		NGX_CONF_FLAG：配置指令可以接受的值是”on”或者”off”，最终会被转成bool值。
		*/
		ngx_conf_set_num_slot,
		NGX_HTTP_LOC_CONF_OFFSET,
		offsetof(ngx_http_configprint_loc_conf_t,testnum),
		NULL
	},
	{
		ngx_string("printsize"),
		NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,/*
		NGX_CONF_FLAG：配置指令可以接受的值是”on”或者”off”，最终会被转成bool值。
		*/
		ngx_conf_set_size_slot,
		NGX_HTTP_LOC_CONF_OFFSET,
		offsetof(ngx_http_configprint_loc_conf_t,testsize),
		NULL
	},
	ngx_null_command
};

static ngx_http_module_t ngx_http_configprint_module_ctx = {
        NULL,                          /* preconfiguration */
        ngx_http_configprint_init,           /* postconfiguration */

        NULL,                          /* create main configuration */
        NULL,                          /* init main configuration */

        NULL,                          /* create server configuration */
        NULL,                          /* merge server configuration */

        ngx_http_configprint_create_loc_conf, /* create location configuration */
        NULL                            /* merge location configuration */
};

ngx_module_t ngx_http_configprint_module = {
        NGX_MODULE_V1,
        &ngx_http_configprint_module_ctx,    /* module context */
        ngx_http_configprint_commands,       /* module directives */
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

static ngx_int_t ngx_http_configprint_init(ngx_conf_t *cf)
{
	ngx_http_handler_pt  *h;
	ngx_http_core_main_conf_t *cmcf;
	cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module); //取得core_module的cf

        h = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers); // 挂载函数到对应处理阶段
        if (h == NULL) {
                return NGX_ERROR;
        }

        *h = ngx_http_configprint_handler; //将函数指针指向handler函数

        return NGX_OK;
}

static void *ngx_http_configprint_create_loc_conf(ngx_conf_t *cf)
{
        ngx_http_configprint_loc_conf_t* local_conf = NULL;
        local_conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_configprint_loc_conf_t));
        if (local_conf == NULL)
        {
                return NULL;
        }

        ngx_str_null(&local_conf->teststr);/*
		conf->teststr.len = 0;
        conf->teststr.data = NULL;
        */
        local_conf->testflag = NGX_CONF_UNSET;
        local_conf->testnum = NGX_CONF_UNSET;
        local_conf->testsize = NGX_CONF_UNSET_SIZE;/*
		这些配置信息一般默认都应该设为一个未初始化的值，针对这个需求，Nginx定义了一系列的宏定义来代表各种配置所对应数据类型的未初始化值，如下：
		#define NGX_CONF_UNSET       -1
		#define NGX_CONF_UNSET_UINT  (ngx_uint_t) -1
		#define NGX_CONF_UNSET_PTR   (void *) -1
		#define NGX_CONF_UNSET_SIZE  (size_t) -1
		#define NGX_CONF_UNSET_MSEC  (ngx_msec_t) -1
        */

        return local_conf;
}

static ngx_int_t ngx_http_configprint_handler(ngx_http_request_t *r)
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
                	<font color=\"red\">URI:</font> %V,  <font color=\"red\">request_line:</font> %V</strong>",
                	&my_cf->teststr, my_cf->testnum, my_cf->testsize, ++ngx_configprint_visited_times, &r->uri, &r->request_line);
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
