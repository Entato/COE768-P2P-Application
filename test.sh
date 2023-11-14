#!/bin/bash

BIN="./bin"
INDEX="${BIN}/index"
PEER="${BIN}/peer"

start_server() {
	echo "Starting index server"
	${SERVER} &
	sleep 2
}

stop_server() {
	pgrep -x "index" > /dev/null
	if [ $? -eq 0 ] 
	then
		pkill -x "index"
		sleep 2
	else
		echo "Index server not running"
	fi
}

peer_test() {
	${peer} &

}


start_server
peer_test
stop_server
