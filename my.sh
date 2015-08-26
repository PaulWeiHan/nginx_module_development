make
killall nginx
sudo rm -r /home/renwh/nginx
make install
cp /home/renwh/nginx.conf.bak /home/renwh/nginx/conf/nginx.conf
