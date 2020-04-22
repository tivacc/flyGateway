PIDS=`ps -ef |grep quectel-CM |grep -v grep | awk '{print $2}'`
if [ "$PIDS" != "" ]; then
echo "quectel-CM is runing!"
else
cd /forlinx/
 ./quectel-CM  & 
fi