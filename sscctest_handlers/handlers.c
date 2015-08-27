
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_http_sscctest_module.h>

//原来定义在ngx_string.c中的函数，为避免加载动态链接库时重定义问题需要加static关键字，定义在这个文件中。
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

static u_char *
ngx_sscc_sprintf_num(u_char *buf, u_char *last, uint64_t ui64, u_char zero,
    ngx_uint_t hexadecimal, ngx_uint_t width)
{
    u_char         *p, temp[NGX_INT64_LEN + 1];
                       /*
                        * we need temp[NGX_INT64_LEN] only,
                        * but icc issues the warning
                        */
    size_t          len;
    uint32_t        ui32;
    static u_char   hex[] = "0123456789abcdef";
    static u_char   HEX[] = "0123456789ABCDEF";

    p = temp + NGX_INT64_LEN;

    if (hexadecimal == 0) {

        if (ui64 <= (uint64_t) NGX_MAX_UINT32_VALUE) {

            /*
             * To divide 64-bit numbers and to find remainders
             * on the x86 platform gcc and icc call the libc functions
             * [u]divdi3() and [u]moddi3(), they call another function
             * in its turn.  On FreeBSD it is the qdivrem() function,
             * its source code is about 170 lines of the code.
             * The glibc counterpart is about 150 lines of the code.
             *
             * For 32-bit numbers and some divisors gcc and icc use
             * a inlined multiplication and shifts.  For example,
             * unsigned "i32 / 10" is compiled to
             *
             *     (i32 * 0xCCCCCCCD) >> 35
             */

            ui32 = (uint32_t) ui64;

            do {
                *--p = (u_char) (ui32 % 10 + '0');
            } while (ui32 /= 10);

        } else {
            do {
                *--p = (u_char) (ui64 % 10 + '0');
            } while (ui64 /= 10);
        }

    } else if (hexadecimal == 1) {

        do {

            /* the "(uint32_t)" cast disables the BCC's warning */
            *--p = hex[(uint32_t) (ui64 & 0xf)];

        } while (ui64 >>= 4);

    } else { /* hexadecimal == 2 */

        do {

            /* the "(uint32_t)" cast disables the BCC's warning */
            *--p = HEX[(uint32_t) (ui64 & 0xf)];

        } while (ui64 >>= 4);
    }

    /* zero or space padding */

    len = (temp + NGX_INT64_LEN) - p;

    while (len++ < width && buf < last) {
        *buf++ = zero;
    }

    /* number safe copy */

    len = (temp + NGX_INT64_LEN) - p;

    if (buf + len > last) {
        len = last - buf;
    }

    return ngx_cpymem(buf, p, len);
}

static u_char *
ngx_sscc_vslprintf(u_char *buf, u_char *last, const char *fmt, va_list args)
{
    u_char                *p, zero;
    int                    d;
    double                 f;
    size_t                 len, slen;
    int64_t                i64;
    uint64_t               ui64, frac;
    ngx_msec_t             ms;
    ngx_uint_t             width, sign, hex, max_width, frac_width, scale, n;
    ngx_str_t             *v;
    ngx_variable_value_t  *vv;

    while (*fmt && buf < last) {

        /*
         * "buf < last" means that we could copy at least one character:
         * the plain character, "%%", "%c", and minus without the checking
         */

        if (*fmt == '%') {

            i64 = 0;
            ui64 = 0;

            zero = (u_char) ((*++fmt == '0') ? '0' : ' ');
            width = 0;
            sign = 1;
            hex = 0;
            max_width = 0;
            frac_width = 0;
            slen = (size_t) -1;

            while (*fmt >= '0' && *fmt <= '9') {
                width = width * 10 + *fmt++ - '0';
            }


            for ( ;; ) {
                switch (*fmt) {

                case 'u':
                    sign = 0;
                    fmt++;
                    continue;

                case 'm':
                    max_width = 1;
                    fmt++;
                    continue;

                case 'X':
                    hex = 2;
                    sign = 0;
                    fmt++;
                    continue;

                case 'x':
                    hex = 1;
                    sign = 0;
                    fmt++;
                    continue;

                case '.':
                    fmt++;

                    while (*fmt >= '0' && *fmt <= '9') {
                        frac_width = frac_width * 10 + *fmt++ - '0';
                    }

                    break;

                case '*':
                    slen = va_arg(args, size_t);
                    fmt++;
                    continue;

                default:
                    break;
                }

                break;
            }


            switch (*fmt) {

            case 'V':
                v = va_arg(args, ngx_str_t *);

                len = ngx_min(((size_t) (last - buf)), v->len);
                buf = ngx_cpymem(buf, v->data, len);
                fmt++;

                continue;

            case 'v':
                vv = va_arg(args, ngx_variable_value_t *);

                len = ngx_min(((size_t) (last - buf)), vv->len);
                buf = ngx_cpymem(buf, vv->data, len);
                fmt++;

                continue;

            case 's':
                p = va_arg(args, u_char *);

                if (slen == (size_t) -1) {
                    while (*p && buf < last) {
                        *buf++ = *p++;
                    }

                } else {
                    len = ngx_min(((size_t) (last - buf)), slen);
                    buf = ngx_cpymem(buf, p, len);
                }

                fmt++;

                continue;

            case 'O':
                i64 = (int64_t) va_arg(args, off_t);
                sign = 1;
                break;

            case 'P':
                i64 = (int64_t) va_arg(args, ngx_pid_t);
                sign = 1;
                break;

            case 'T':
                i64 = (int64_t) va_arg(args, time_t);
                sign = 1;
                break;

            case 'M':
                ms = (ngx_msec_t) va_arg(args, ngx_msec_t);
                if ((ngx_msec_int_t) ms == -1) {
                    sign = 1;
                    i64 = -1;
                } else {
                    sign = 0;
                    ui64 = (uint64_t) ms;
                }
                break;

            case 'z':
                if (sign) {
                    i64 = (int64_t) va_arg(args, ssize_t);
                } else {
                    ui64 = (uint64_t) va_arg(args, size_t);
                }
                break;

            case 'i':
                if (sign) {
                    i64 = (int64_t) va_arg(args, ngx_int_t);
                } else {
                    ui64 = (uint64_t) va_arg(args, ngx_uint_t);
                }

                if (max_width) {
                    width = NGX_INT_T_LEN;
                }

                break;

            case 'd':
                if (sign) {
                    i64 = (int64_t) va_arg(args, int);
                } else {
                    ui64 = (uint64_t) va_arg(args, u_int);
                }
                break;

            case 'l':
                if (sign) {
                    i64 = (int64_t) va_arg(args, long);
                } else {
                    ui64 = (uint64_t) va_arg(args, u_long);
                }
                break;

            case 'D':
                if (sign) {
                    i64 = (int64_t) va_arg(args, int32_t);
                } else {
                    ui64 = (uint64_t) va_arg(args, uint32_t);
                }
                break;

            case 'L':
                if (sign) {
                    i64 = va_arg(args, int64_t);
                } else {
                    ui64 = va_arg(args, uint64_t);
                }
                break;

            case 'A':
                if (sign) {
                    i64 = (int64_t) va_arg(args, ngx_atomic_int_t);
                } else {
                    ui64 = (uint64_t) va_arg(args, ngx_atomic_uint_t);
                }

                if (max_width) {
                    width = NGX_ATOMIC_T_LEN;
                }

                break;

            case 'f':
                f = va_arg(args, double);

                if (f < 0) {
                    *buf++ = '-';
                    f = -f;
                }

                ui64 = (int64_t) f;
                frac = 0;

                if (frac_width) {

                    scale = 1;
                    for (n = frac_width; n; n--) {
                        scale *= 10;
                    }

                    frac = (uint64_t) ((f - (double) ui64) * scale + 0.5);

                    if (frac == scale) {
                        ui64++;
                        frac = 0;
                    }
                }

                buf = ngx_sprintf_num(buf, last, ui64, zero, 0, width);

                if (frac_width) {
                    if (buf < last) {
                        *buf++ = '.';
                    }

                    buf = ngx_sprintf_num(buf, last, frac, '0', 0, frac_width);
                }

                fmt++;

                continue;

#if !(NGX_WIN32)
            case 'r':
                i64 = (int64_t) va_arg(args, rlim_t);
                sign = 1;
                break;
#endif

            case 'p':
                ui64 = (uintptr_t) va_arg(args, void *);
                hex = 2;
                sign = 0;
                zero = '0';
                width = NGX_PTR_SIZE * 2;
                break;

            case 'c':
                d = va_arg(args, int);
                *buf++ = (u_char) (d & 0xff);
                fmt++;

                continue;

            case 'Z':
                *buf++ = '\0';
                fmt++;

                continue;

            case 'N':
#if (NGX_WIN32)
                *buf++ = CR;
                if (buf < last) {
                    *buf++ = LF;
                }
#else
                *buf++ = LF;
#endif
                fmt++;

                continue;

            case '%':
                *buf++ = '%';
                fmt++;

                continue;

            default:
                *buf++ = *fmt++;

                continue;
            }

            if (sign) {
                if (i64 < 0) {
                    *buf++ = '-';
                    ui64 = (uint64_t) -i64;

                } else {
                    ui64 = (uint64_t) i64;
                }
            }

            buf = ngx_sprintf_num(buf, last, ui64, zero, hex, width);

            fmt++;

        } else {
            *buf++ = *fmt++;
        }
    }

    return buf;
}

static u_char * ngx_cdecl
ngx_sscc_sprintf(u_char *buf, const char *fmt, ...)
{
    u_char   *p;
    va_list   args;

    va_start(args, fmt);
    p = ngx_sscc_vslprintf(buf, (void *) -1, fmt, args);
    va_end(args);

    return p;
}


ngx_int_t ngx_http_ssscctest_realhandler_test(ngx_http_sscctest_request_t *req, ngx_http_sscctest_response_t *resp)
{
    FILE *fp=fopen("/home/renwh/handler_function_debug","rw");
    if(  fp == NULL ){
            printf("error");
        }
    fprintf(fp,"ngx_http_ssscctest_realhandler_test is called\n");
    fclose(fp);
	ngx_int_t rc;
	ngx_buf_t *b;
	u_char ngx_my_string[1024] = {0};
	ngx_uint_t content_length = 0;
    //printf("hhhh\n");
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

    ngx_sscc_sprintf(ngx_my_string, "<font color=\"red\">URI:</font> %V,  <font color=\"red\">method:</font> %V, <font color=\"red\">remoteAddr:</font> %V : %D",
                	&req->uri, &req->method, &req->remoteAddr, req->remotePort); 

    content_length = ngx_strlen(ngx_my_string);

    /* we response to 'GET' and 'HEAD' requests only */
        if ((ngx_sscc_strncasecmp(req->method.data,(u_char *)"GET",3) && ngx_sscc_strncasecmp(req->method.data,(u_char *)"HEAD",4)) != 0) {
                return NGX_HTTP_NOT_ALLOWED;
        }

        

        if (rc != NGX_OK) {
                return rc;
        }
		/* set the 'Content-type' header */
        /*
         *r->headers_out.content_type.len = sizeof("text/html") - 1;
         *r->headers_out.content_type.data = (u_char *)"text/html";
         */
        ngx_str_set(&resp->headers_out.content_type, "text/html");
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
        return NGX_OK;
}
