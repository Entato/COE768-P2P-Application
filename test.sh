#!/bin/bash

BIN="./bin"
INDEX="${BIN}/index"
PEER="${BIN}/peer"

PIPE1="peer1.pipe"
PIPE2="peer2.pipe"

start_server() {
	echo "Starting index server"
	${INDEX} &
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

peer_create() {
	mkfifo ${PIPE1}
	${PEER} < ${PIPE1} &
	sleep infinity > ${PIPE1}&

	mkfifo ${PIPE2}
	${PEER} < ${PIPE2} &
	sleep infinity > ${PIPE2}&

	sleep 2
}

stop_peers() {
	pgrep -x "peer" > /dev/null
	if [ $? -eq 0 ]
	then
		pkill -x "peer"
		sleep 2
	else
		echo "No peers running"
	fi
	rm -f *.pipe
}

send_input() {
	echo $2 | tee -a $1
}

p2p_test() {
	send_input ${PIPE1} "1"
	send_input ${PIPE1} "name"
	send_input ${PIPE1} "content"

	sleep 1

	send_input ${PIPE2} "2"
	send_input ${PIPE2} "content"
}


start_server
peer_create
p2p_test
stop_peers
stop_server
