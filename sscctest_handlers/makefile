CORE_DIR=/home/renwh/src/nginx-1.9.2/src/core/
EVENT_DIR=/home/renwh/src/nginx-1.9.2/src/event/
EVEVT_MODULE_DIR=/home/renwh/src/nginx-1.9.2/src/event/modules/
HTTP_DIR=/home/renwh/src/nginx-1.9.2/src/http/
HTTP_MODULE_DIR=/home/renwh/src/nginx-1.9.2/src/http/modules/
HTTP_MODULE_PERL_DIR=/home/renwh/src/nginx-1.9.2/src/http/modules/perl/
OS_UNIX_DIR=/home/renwh/src/nginx-1.9.2/src/os/unix/
STREAM_DIR=/home/renwh/src/nginx-1.9.2/src/stream/
SSCCTEST_DIR=/home/renwh/src/nginx-1.9.2/sscctest/
OBJ_DIR=/home/renwh/src/nginx-1.9.2/objs/

mylib.so:
	gcc -fpic -shared -g handlers.c -o mylib.so -I $(CORE_DIR) -I $(HTTP_DIR) -I $(SSCCTEST_DIR) -I $(EVENT_DIR) \
	-I $(EVEVT_MODULE_DIR) -I $(HTTP_MODULE_DIR) -I $(HTTP_MODULE_PERL_DIR) -I $(OS_UNIX_DIR) \
	-I $(STREAM_DIR) -I $(OBJ_DIR)
	
.PHONY: clean

clean:
	rm *.so