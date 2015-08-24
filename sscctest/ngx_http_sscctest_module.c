/*
	Copyright (C) Paul 
	rwhsysu@163.com

	for nginx.conf 

	location /sscctest {
		flag;
	} 


*/

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <dlfcn.h> //显式调用动态链接库文件需要加载的头文件
#include <ngx_http_sscctest_module.h>


static char * ngx_http_sscctest(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);//handler挂载函数, 按需挂载；

static void * ngx_http_sscctest_create_loc_conf(ngx_conf_t *cf);//打开handler函数定义文件，调用所有函数，
																//挂载到loc_conf的handlers的数组中

static ngx_int_t ngx_http_sscctest_handler(ngx_http_request_t *r); //handler 函数定义

static ngx_int_t ngx_get_args_array(ngx_http_request_t *r, ngx_array_t *a);

void *pdlHandle;//动态链接库文件解析句柄
char *pszErr;//动态链接库解析错误指针


static ngx_command_t ngx_http_sscctest_commands[]={
	{
		ngx_string("flag"),
		NGX_HTTP_LOC_CONF|NGX_CONF_FLAG,/*
		NGX_CONF_FLAG：配置指令可以接受的值是”on”或者”off”，最终会被转成bool值。
		*/
		ngx_http_sscctest,
		NGX_HTTP_LOC_CONF_OFFSET,
		0,
		NULL
	},
	ngx_null_command
};

static ngx_http_module_t ngx_http_sscctest_module_ctx = {
        NULL,                          /* preconfiguration */
        NULL,                          /* postconfiguration */

        NULL,                          /* create main configuration */
        NULL,                          /* init main configuration */

        NULL,                          /* create server configuration */
        NULL,                          /* merge server configuration */

        ngx_http_sscctest_create_loc_conf, /* create location configuration */
        NULL                            /* merge location configuration */
};

ngx_module_t ngx_http_sscctest_module = {
        NGX_MODULE_V1,
        &ngx_http_sscctest_module_ctx,    /* module context */
        ngx_http_sscctest_commands,       /* module directives */
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

static ngx_int_t ngx_get_args_array(ngx_http_request_t *r, ngx_array_t *a)
{
    u_char  *p, *last;
    ngx_table_elt_t *k2v;

    if (r->args.len == 0) {
        return NGX_DECLINED;
    }

    p = r->args.data;
    last = p + r->args.len;
    

    for ( /* void */ ; p < last; p++) {

        if (p == r->args.data || *(p - 1) == '&') {

            k2v = (ngx_table_elt_t *) ngx_array_push(a);

            k2v->key->data = p;
            p = ngx_strlchr(p, last, '=');
            k2v->key->len = p - k2v->key->data;
            p++;
            k2v->value->data =p; 
            p = ngx_strlchr(p, last, '&');

            if (p == NULL) {
                p = r->args.data + r->args.len;
            }
            k2v->value->len = p - k2v->value->data;
        }
    }
    return NGX_OK;
}

static char * ngx_http_sscctest(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) //handler挂载函数, 按需挂载
{
	ngx_http_core_loc_conf_t *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_sscctest_handler;

    return NGX_CONF_OK;
}

static void *ngx_http_sscctest_create_loc_conf(ngx_conf_t *cf)
{
        FILE *fp;
        ngx_uint_t maxnum_of_handlers = 100;
        ngx_http_sscctest_loc_conf_t* local_conf = NULL;
        local_conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_sscctest_loc_conf_t));
        if (local_conf == NULL)
        {
                ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "ngx_http_sscctest_create_loc_conf error!\n");
                return NULL;
        }
        ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "ngx_http_sscctest_create_loc_conf is called!\n");
        /*
        创建unit_t数组，储存args——handler_pt对。
		typedef struct ngx_array_s       ngx_array_t;
		struct ngx_array_s {
		    void        *elts;
		    ngx_uint_t   nelts;
		    size_t       size;
		    ngx_uint_t   nalloc;
		    ngx_pool_t  *pool;
		};
		elts:	指向实际的数据存储区域。
		nelts:	数组实际元素个数。
		size:	数组单个元素的大小，单位是字节。
		nalloc:	数组的容量。表示该数组在不引发扩容的前提下，可以最多存储的元素的个数。当nelts增长到达nalloc 时，如果再往此数组中存储元素，则会引发数组的扩容。数组的容量将会扩展到原有容量的2倍大小。实际上是分配新的一块内存，新的一块内存的大小是原有内存大小的2倍。原有的数据会被拷贝到新的一块内存中。
		pool:	该数组用来分配内存的内存池。

		ngx_array_t *ngx_array_create(ngx_pool_t *p, ngx_uint_t n, size_t size);
		创建一个新的数组对象，并返回这个对象。
		p:	数组分配内存使用的内存池；
		n:	数组的初始容量大小，即在不扩容的情况下最多可以容纳的元素个数。
		size:	单个元素的大小，单位是字节。

        void ngx_array_destroy(ngx_array_t *a);
        销毁该数组对象，并释放其分配的内存回内存池。

        void *ngx_array_push(ngx_array_t *a);
        在数组a上新追加一个元素，并返回指向新元素的指针。需要把返回的指针使用类型转换，转换为具体的类型，然后再给新元素本身或者是各字段（如果数组的元素是复杂类型）赋值。

        void *ngx_array_push_n(ngx_array_t *a, ngx_uint_t n);
        在数组a上追加n个元素，并返回指向这些追加元素的首个元素的位置的指针。

        static ngx_inline ngx_int_t ngx_array_init(ngx_array_t *array, ngx_pool_t *pool, ngx_uint_t n, size_t size);
        如果一个数组对象是被分配在堆上的，那么当调用ngx_array_destroy销毁以后，如果想再次使用，就可以调用此函数。

        如果一个数组对象是被分配在栈上的，那么就需要调用此函数，进行初始化的工作以后，才可以使用。

        注意事项: 由于使用ngx_palloc分配内存，数组在扩容时，旧的内存不会被释放，会造成内存的浪费。因此，最好能提前规划好数组的容量，在创建或者初始化的时候一次搞定，避免多次扩容，造成内存浪费。
        */
        local_conf->handlers = ngx_array_create(cf->pool, maxnum_of_handlers, sizeof(unit_t));
        /*
		打开文件,读取配置
        */
        if(fp=fopen("/home/renwh/sscctest","r") == NULL){
            ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "ngx_http_sscctest_create_loc_conf: openfile error.\n");
        }
        /*
        打开动态链接库文件，准备加载函数指针
        */
        pdlHandle = dlopen("./mylib.so", RTLD_LAZY); // RTLD_LAZY 延迟加载
        pszErr = dlerror();
        if( !pdlHandle || pszErr )
        {
            ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "ngx_http_sscctest_create_loc_conf: Load mylib failed!\n");
        }
        /*
        解析配置文件：将args和函数指针加载到handlers数组中
        */
        int ch,j=0;
        u_char agr[1024];
        u_char handler_name[1024];
        while(1)//判断文件是否结束，结束为1，未结束为0
        {
            //!eof
            ch=fgetc(fp);
            if(ch==EOF) {
                break;
            }
            else if(ch=='\n'){
                continue;
            }
            else if(ch==' '){
                continue;
            }
            else if(ch=='#'){
                while(ch=fgetc(fp) != '#'){
                    agr[j]=ch;
                    j++;
                }
                //set args
                unit_t * unit=(unit_t *)ngx_array_push(local_conf->handlers);
                unit->arg.len = (size_t)j;
                //分配内存存储args
                u_char* temp=(u_char *)ngx_pcalloc(cf->pool, local_conf->handlers.arg.len);
                unit->arg.data = temp;
                memcpy(temp,agr,local_conf->handlers[i].arg.len);
                // j=0;
                // while(j<=local_conf->handlers.arg.len){
                //     *temp = arg[j];
                //     temp++;j++;
                // }
                //i++;
                j=0;
                continue;
            }
            else if(ch=='$'){
                while(ch=fgetc(fp) != '$'){
                    handler_name[j]=ch;
                    j++;
                }
                handler_name[j]='\0';
                unit->handler_func = dlsym(pdlHandle, handler_name); // 定位动态链接库中的函数
                j=0;
            }
        }
        fp.close();
        return local_conf;
}

static ngx_int_t ngx_http_sscctest_handler(ngx_http_request_t *r)
{
	ngx_int_t rc;
	ngx_buf_t *b;
	ngx_chain_t out;
	ngx_http_sscctest_loc_conf_t *my_cf;
    ngx_array_t *args;
	u_char ngx_my_string[1024] = {0};
	ngx_uint_t content_length = 0;
	ngx_http_sscctest_request_t sscc_C_request;
    ngx_http_sscctest_response_t sscc_C_response;

	ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "ngx_http_sscctest_handler is called!");

    /**************************    sscc_C_request结构体cookies赋值    *******************************/
    sscc_C_request.cookies = r->headers_in.cookies.elts;

    /**************************    sscc_C_request结构体query赋值       *******************************/
    args = ngx_array_create(cf->pool, maxnum_of_handlers, sizeof(ngx_table_elt_t));
    // static ngx_int_t ngx_get_args_array(ngx_http_request_t *r, ngx_array_t *a)
    // 可以拿HTTP GET的参数。
    // 第一个参数是ngx_http_request_t；
    // 第二个参数是已经创建的用于存储args的key value对的数组指针
    rc = ngx_get_args_array(r,args)；
    if(rc != NGX_OK){
        ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "ngx_get_args_array is wrong!");
    }
    sscc_C_request.query = args->elts;

    /**************************    sscc_C_request结构体remoteAddr赋值      *******************************/
    struct sockaddr_in *ip = (struct sockaddr_in *) (r->connection->sockaddr); //inet_ntoa(ip->sin_addr) , ntohs(ip->sin_port)
    sscc_C_request.remoteAddr = r->connection->addr_text;

    /**************************    sscc_C_request结构体remotePort赋值      *******************************/
    sscc_C_request.remotePort = ntohs(ip->sin_port); // 大小端转化

    /**************************    sscc_C_request结构体 method 赋值      *******************************/
    sscc_C_request.method = r->method_name;

    /**************************    sscc_C_request结构体 uri 赋值      *******************************/
    sscc_C_request.uri = r->uri;
    /**************************    sscc_C_request结构体 httpVersionMajor 赋值      *******************************/
    // nginx 宏定义如下：
    // #define NGX_HTTP_VERSION_9                 9
    // #define NGX_HTTP_VERSION_10                1000
    // #define NGX_HTTP_VERSION_11                1001
    if(r->http_version == NGX_HTTP_VERSION_9){
        sscc_C_request.httpVersionMajor = 0;
        sscc_C_request.httpVersionMinor = 9;
    }
    if(r->http_version == NGX_HTTP_VERSION_10){
        sscc_C_request.httpVersionMajor = 1;
        sscc_C_request.httpVersionMinor = 0;
    }
    if(r->http_version == NGX_HTTP_VERSION_11){
        sscc_C_request.httpVersionMajor = 1;
        sscc_C_request.httpVersionMinor = 1;
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
                	&my_cf->teststr, my_cf->testnum, my_cf->testsize, ++ngx_sscctest_visited_times, &r->uri, r->request);
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
