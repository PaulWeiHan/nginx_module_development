/*
	Copyright (C) Paul 
	rwhsysu@163.com
*/

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

//static ngx_int_t ngx_http_sscctest_init(ngx_conf_t *cf); //handler挂载函数,  content phase handlers

static char * ngx_http_sscctest(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static void * ngx_http_sscctest_create_loc_conf(ngx_conf_t *cf);//打开handler函数定义文件，调用所有函数，
																//挂载到loc_conf的handlers的数组中

static ngx_int_t ngx_http_sscctest_handler(ngx_http_request_t *r); //handler 函数定义

typedef struct nax_http_sscc_request_s ngx_http_sscc_request_t;
struct ngx_http_sscc_request_s
{

};

typedef struct nax_http_sscc_response_s ngx_http_sscc_response_t;
struct ngx_http_sscc_response_s
{

};

typedef struct{

}

//真正处理函数
typedef ngx_int_t (*ngx_http_real_handler_pt)(ngx_http_sscc_request_t *r);

typedef struct 
{
	/* data */
	ngx_array_t	handlers;
	ngx_http_sscc_request_t request;
	ngx_http_sscc_response_t response;

}ngx_http_sscc_loc_conf_t;