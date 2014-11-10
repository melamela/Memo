DEFINE=200

log=ip.list

grep 'Failed password' /var/log/secure |awk '{print $(NF-3)}' |sort |uniq -c > $log

for i in `cat $log|awk '{print $2}'`
do
		NUM=`grep $i $log|awk '{print $1}'`
		echo $i $NUM
        if [ $NUM -gt $DEFINE ] 
        then
        	echo "WARNING"
        	grep $i /etc/hosts.deny > /dev/null
         	if [ $? -gt 0 ]
            then
            echo "ALL:$i" >> /etc/hosts.deny
            iptables -I INPUT -s $i -j DROP
            fi
        fi
done