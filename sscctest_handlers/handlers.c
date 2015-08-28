
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <stdio.h>
#include <ngx_http_sscctest_module.h>

//原来定义在ngx_string.c中的函数，为避免加载动态链接库时重定义问题需要加static关键字，定义在这个文件中。

#define ngx_sscc_cpymem(dst, src, n)   (((u_char *) memcpy(dst, src, n)) + (n))
#define ngx_sscc_min(val1, val2)  ((val1 > val2) ? (val2) : (val1))
#define ngx_sscc_strlen(s)       strlen((const char *) s)
#define ngx_sscc_str_set(str, text)                                               \
    (str)->len = sizeof(text) - 1; (str)->data = (u_char *) text

static ngx_int_t
ngx_sscc_strncasecmp(u_char *s1, u_char *s2, size_t n)
{
    ngx_uint_t  c1, c2;

    while (n) {
        c1 = (ngx_uint_t) *s1++;
        c2 = (ngx_uint_t) *s2++;

        c1 = (c1 >= 'A' && c1 <= 'Z') ? (c1 | 0x20) : c1;
        c2 = (c2 >= 'A' && c2 <= 'Z') ? (c2 | 0x20) : c2;

        if (c1 == c2) {

            if (c1) {
                n--;
                continue;
            }

            return 0;
        }

        return c1 - c2;
    }

    return 0;
}


ngx_int_t ngx_http_ssscctest_realhandler_test(ngx_http_sscctest_request_t *req, ngx_http_sscctest_response_t *resp)
{
   // FILE *fp=fopen("/home/renwh/handler.log","w+");
    //if(  fp == NULL ){
    //    return NGX_HTTP_NOT_ALLOWED;
    //}
    //fprintf(fp,"ngx_http_ssscctest_realhandler_test is called\n");
    
	ngx_int_t rc;
	ngx_buf_t *b;
	u_char *ngx_my_string = malloc(1024);
	ngx_uint_t content_length = 0;
    
    memcpy(ngx_my_string, req->remoteAddr->data, req->remoteAddr->len);
    ngx_my_string[req->remoteAddr->len] = '\0';
    content_length = ngx_strlen(ngx_my_string);
    //fprintf(fp, "%s\n", ngx_my_string);
     /*
    ngx_sscc_sprintf(ngx_my_string, "<font color=\"red\">URI:</font> %V,  <font color=\"red\">method:</font> %V, <font color=\"red\">remoteAddr:</font> %V : %D",
                	&req->uri, &req->method, &req->remoteAddr, req->remotePort); 
    */ 

    // we response to 'GET' and 'HEAD' requests only 
        if ((ngx_sscc_strncasecmp(req->method.data,(u_char *)"GET",3) && ngx_sscc_strncasecmp(req->method.data,(u_char *)"HEAD",4)) != 0) {
                return NGX_HTTP_NOT_ALLOWED;
        }
		/* set the 'Content-type' header */
        /*
         *r->headers_out.content_type.len = sizeof("text/html") - 1;
         *r->headers_out.content_type.data = (u_char *)"text/html";
         */
        ngx_sscc_str_set(&resp->headers_out.content_type, "text/html");
        resp->headers_out.status = NGX_HTTP_OK;
        resp->headers_out.content_length_n = content_length;

        /* allocate a buffer for your response body */
        b = malloc(sizeof(ngx_buf_t));
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
        resp->buffers.buf = b;
        resp->buffers.next = NULL;
        //fclose(fp);
        return NGX_OK;
}
