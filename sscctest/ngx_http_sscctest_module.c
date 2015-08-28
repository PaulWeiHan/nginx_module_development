/*
	Copyright (C) Paul 
	rwhsysu@163.com

    /home/renwh/sscctest
    /home/renwh/mylib.so
    /nginx stop的时候添加一行关闭动态链接库文件的程序：未添加

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

static ngx_int_t ngx_get_args_array(ngx_http_request_t *r, ngx_array_t *a); //args数组赋值

void *pdlHandle;//动态链接库文件解析句柄
char *pszErr;//动态链接库解析错误指针

typedef struct 
{
    /* data */
    ngx_array_t* handlers;

}ngx_http_sscctest_loc_conf_t;

static ngx_command_t ngx_http_sscctest_commands[]={
	{
		ngx_string("flag"),
		NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,/*
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

    ngx_table_elt_t *test;
    

    if (r->args.len == 0) {
        return NGX_DECLINED;
    }
    ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "ngx_get_args_array is called! init  \n");
    p = r->args.data;
    last = p + r->args.len;
    

    for ( /* void */ ; p < last; p++) {

        if (p == r->args.data || *(p - 1) == '&') {

            ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "ngx_get_args_array is called! a->nelts:%d ; a->elts:%d \n",a->nelts,a->elts);
            //k2v = ngx_array_push(a);
            k2v = (ngx_table_elt_t *) a->elts + a->size * a->nelts;
            a->nelts++;
            ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "ngx_get_args_array is called! k2v:%l ; a->elts:%l \n",k2v,a->elts);
            k2v->key.data = p;
            p = ngx_strlchr(p, last, '=');
            k2v->key.len = p - k2v->key.data;
            p++;
            k2v->value.data =p; 
            p = ngx_strlchr(p, last, '&');

            if (p == NULL) {
                p = r->args.data + r->args.len;
            }
            k2v->value.len = p - k2v->value.data;
        }
    }
    ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "ngx_get_args_array is called! k2v->value:%V ; k2v->size:%D \n",&k2v->value,sizeof(ngx_table_elt_t));
    test = a->elts; 
    ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "ngx_get_args_array is called! %z %D %l test2->key:%V ; test2->value:%V \n",a->size,a->nelts,test,&test[0].key,&test[0].value);

    return NGX_OK;
}

static char * ngx_http_sscctest(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) //handler挂载函数, 按需挂载
{
	ngx_http_core_loc_conf_t *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_sscctest_handler;

    return NGX_CONF_OK;
}
//ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "hello_string:%s", local_conf->hello_string.data)
static void *ngx_http_sscctest_create_loc_conf(ngx_conf_t *cf)
{
        
        ngx_uint_t maxnum_of_handlers = 20;
        ngx_http_sscctest_loc_conf_t* local_conf = NULL;
        local_conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_sscctest_loc_conf_t));
        if (local_conf == NULL)
        {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "ngx_http_sscctest_create_loc_conf error!\n");
                return NULL;
        }
        //ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,"ngx_http_sscctest_create_loc_conf is called!\n");
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
        FILE *fp=fopen("/home/renwh/sscctest","r");
        if(  fp == NULL ){
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "ngx_http_sscctest_create_loc_conf: openfile error.\n");
        }
        /*
        打开动态链接库文件，准备加载函数指针
        */
        pdlHandle = dlopen("/home/renwh/mylib.so", RTLD_LAZY); // RTLD_LAZY 延迟加载
        pszErr = dlerror();
        if( !pdlHandle || pszErr )
        {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "ngx_http_sscctest_create_loc_conf: Load mylib failed!\n");
        }
        /*
        解析配置文件：将args和函数指针加载到handlers数组中
        */
        int ch,j=0;
        unit_t *unit =NULL;
        u_char agr[1024];
        u_char handler_name[1024];
        while(1)//判断文件是否结束，结束为1，未结束为0
        {
            //!eof
            ch = fgetc(fp);
            if(ch == EOF) {
                break;
            }
            else if(ch == '\n'){
                continue;
            }
            else if(ch == ' '){
                continue;
            }
            else if(ch == '#'){
                ch = fgetc(fp);
                while(  ch != '#' ){
                    agr[j]=ch;
                    j++;
                    ch = fgetc(fp);
                }
                //set args
                unit=(unit_t *)ngx_array_push(local_conf->handlers);
                unit->arg.len = (size_t)j;
                //分配内存存储args
                u_char* temp=(u_char *)ngx_pcalloc(cf->pool, unit->arg.len);
                unit->arg.data = temp;
                memcpy(temp,agr,unit->arg.len);
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
                ch=fgetc(fp);
                while( ch != '$' ){
                    handler_name[j]=ch;
                    j++;
                    ch=fgetc(fp);
                }
                handler_name[j]='\0';
                unit->handler_func = dlsym(pdlHandle, (char *)handler_name); // 定位动态链接库中的函数
                j=0;
                continue;
            }
        }
        fclose(fp);
        return local_conf;
}

static ngx_int_t ngx_http_sscctest_handler(ngx_http_request_t *r)
{
	ngx_int_t rc;
	//ngx_buf_t *b;
    unit_t *unit;
	ngx_chain_t out;
    ngx_str_t handler_name;
	ngx_http_sscctest_loc_conf_t *my_cf;
    ngx_array_t *args;
	//u_char ngx_my_string[1024] = {0};
	//ngx_uint_t content_length = 0;
	ngx_http_sscctest_request_t sscc_C_request;
    ngx_http_sscctest_response_t sscc_C_response;

	ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "ngx_http_sscctest_handler is called!\n");

    my_cf = ngx_http_get_module_loc_conf(r,ngx_http_sscctest_module);

    /**************************    sscc_C_request结构体cookies赋值    *******************************/
    sscc_C_request.cookies = r->headers_in.cookies.elts;

    /**************************    sscc_C_request结构体query赋值       *******************************/
    args = ngx_array_create(r->pool, 20, sizeof(ngx_table_elt_t));
    // static ngx_int_t ngx_get_args_array(ngx_http_request_t *r, ngx_array_t *a)
    // 可以拿HTTP GET的参数。
    // 第一个参数是ngx_http_request_t；
    // 第二个参数是已经创建的用于存储args的key value对的数组指针
    ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "ngx_get_args_array is init!\n");
    rc = ngx_get_args_array(r,args);
    if(rc != NGX_OK){
        ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "ngx_get_args_array is error!\n");
    }
    ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "ngx_get_args_array is end!\n");
    sscc_C_request.query = args->elts;

    /**************************    sscc_C_request结构体remoteAddr赋值      *******************************/
    struct sockaddr_in *ip = (struct sockaddr_in *) (r->connection->sockaddr); //inet_ntoa(ip->sin_addr) , ntohs(ip->sin_port)
    sscc_C_request.remoteAddr = &r->connection->addr_text;

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
    /**************************    sscc_C_request结构体 headers_in 赋值      *******************************/
    //复制了一个ngx_http_headers_in_t结构体
    sscc_C_request.headers_in = r->headers_in;

    /*************************  根据传入参数中的handler_name的值，决定调用的real_handler  **********************/
    ngx_table_elt_t *args_unit = args->elts;

    
    ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "real_handler name compare\n");
    ngx_uint_t i;
    for(i=0; i<=args->nelts; i++){
        ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "real_handler name compare:for len=%z key:%V value:%V \n",args_unit[i].key.len,&args_unit[i].key,&args_unit[i].value);
        if( ngx_strncasecmp( args_unit[i].key.data, (u_char *) "handler_name", args_unit[i].key.len) == 0 ){
            ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "real_handler name compare:if\n");
            handler_name = args_unit[i].value;
            ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "real_handler is %V\n",&handler_name);
            break;
        }
    }

    unit = my_cf->handlers->elts;
    for(i=0; i<=my_cf->handlers->nelts; i++){
        if(ngx_strncasecmp(unit[i].arg.data, handler_name.data,unit[i].arg.len) == 0 ){
            //调用real_handler函数
            ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "handlers %i arg  is %V\n",i,&handler_name);
            rc = unit[i].handler_func(&sscc_C_request, &sscc_C_response);
            if(rc!=NGX_OK) {
                return rc;
            }
            ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "handler_func out! remoteAddr:%s \n ", sscc_C_response.buffers.buf->pos);
            //ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "handler_func out! headers_out.content_type:%V \n ", &sscc_C_response.headers_out.content_type);
            break;
        }
    }
    /* discard request body, since we don't need it here */
    rc = ngx_http_discard_request_body(r);
    if (rc != NGX_OK) {
            return rc;
    }

    r->headers_out = sscc_C_response.headers_out;
    //ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "r->headers_out.content_type\n");
    ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "r->headers_out.content_type:%V \n ", &r->headers_out.content_type);
    ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "r->headers_out.content_length_n:%d \n ", r->headers_out.content_length_n);
    /* send the header only, if the request type is http 'HEAD' */
    if (r->method == NGX_HTTP_HEAD) {
            return ngx_http_send_header(r);
    }

    out= sscc_C_response.buffers;
    /* send the headers of your response */
    ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "ngx_http_send_header ahead!\n ");

    rc = ngx_http_send_header(r);

    ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "ngx_http_send_header out!\n ");

    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
            return rc;
    }

    /* send the buffer chain of your response */
    return ngx_http_output_filter(r, &out);
}
