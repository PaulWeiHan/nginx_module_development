/*
	Copyright (C) Paul 
	rwhsysu@163.com
*/
#ifndef _NGX_HTTP_SSCCTEST_MODULE_H_INCLUDED_
#define _NGX_HTTP_SSCCTEST_MODULE_H_INCLUDED_

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct ngx_http_sscctest_request_s ngx_http_sscctest_request_t;
struct ngx_http_sscctest_request_s
{
	ngx_table_elt_t*	query; 
	ngx_str_t*	        remoteAddr;
	unsigned short		remotePort;
	ngx_str_t			method;
	ngx_str_t			uri;
	ngx_int_t			httpVersionMajor;
	ngx_int_t			httpVersionMinor;
	ngx_http_headers_in_t	headers_in;			

};

typedef struct ngx_http_sscctest_response_s ngx_http_sscctest_response_t;
struct ngx_http_sscctest_response_s
{
	enum StatusType
    {
        ok = 200,
        created = 201,
        accepted = 202,
        no_content = 204,
        multiple_choices = 300,
        moved_permanently = 301,
        moved_temporarily = 302,
        not_modified = 304,
        bad_request = 400,
        unauthorized = 401,
        forbidden = 403,
        not_found = 404,
        internal_server_error = 500,
        not_implemented = 501,
        bad_gateway = 502,
        service_unavailable = 503
    } status;
    ngx_http_headers_out_t *headers_out;
    ngx_str_t content;
    ngx_chain_t buffers;
};

//真正处理函数
typedef ngx_int_t (*ngx_http_real_handler_pt)(ngx_http_sscctest_request_t *req, ngx_http_sscctest_response_t *resp);

typedef struct{
	ngx_str_t arg;
	ngx_http_real_handler_pt handler_func;
}unit_t;

// typedef struct 
// {
//     /* data */
//     ngx_array_t* handlers;

// }ngx_http_sscctest_loc_conf_t;




#endif /*_NGX_HTTP_SSCCTEST_MODULE_H_INCLUDED_*/