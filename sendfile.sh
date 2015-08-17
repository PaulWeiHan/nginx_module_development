./configure --prefix=/home/renwh/nginx --add-module=/home/renwh/src/nginx-1.9.2/sendfile/ 
make
sudo rm -r /home/renwh/nginx
make install
#cp /home/renwh/nginx.conf.bak /home/renwh/nginx/conf/nginx.conf
