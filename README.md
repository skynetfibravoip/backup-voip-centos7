# backup-voip-centos7
yum install -y git
git clone git@github.com:seu_usuario/seu_repositorio.git /backup/voip

rsync -avz /backup/voip/etc/ /etc/
rsync -avz /backup/voip/var_lib/ /var/lib/
rsync -avz /backup/voip/usr_src/ /usr/src/
rsync -avz /backup/voip/home/ /home/


mysql -u root -p < /backup/voip/banco.sql

psql -U postgres < /backup/voip/banco.sql


chown -R root:root /etc/
chown -R mysql:mysql /var/lib/mysql/
chown -R postgres:postgres /var/lib/postgresql/
chown -R asterisk:asterisk /var/lib/asterisk/


systemctl restart asterisk
systemctl restart mysql
systemctl restart httpd



Usar a versao 
Asterisk 13.30.0
