intIP=`hostname -Itr -d [' ']`
mac_address=$(ifconfig eth0 grep -oE '([0-9A-Fa-f]{2}){5}[0-9A-Fa-f]{2}'  head -n 1  tr '' '-'  tr [lower] [upper])
port=10000
URL='<Domain>/caminfotodb.phpintIP='$intIP'&&port='$port'&&macAddr='$mac_address
echo
curl -X GET $URL
echo

homegstgst-rtsp-server-1.19.2buildexamplestest-launch '( v4l2src device=devvideo0 ! videox-h264, width=1280, height=720, framerate=301 ! h264parse config-interval=1 ! rtph264pay name=pay0 pt=96 )'