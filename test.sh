#!/bin/bash

trap "kill 0" SIGINT

BIN="./bin"
INDEX="${BIN}/index"
PEER="${BIN}/peer"
TESTDIR="./test"

PIPE1="peer1.pipe"
PIPE2="peer2.pipe"
PIPE3="peer3.pipe"

start_server() {
	echo "Starting index server"
	mkdir -p ${TESTDIR}/index
	(cd ${TESTDIR}/index && ../../${INDEX} &)
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
	mkdir -p ${TESTDIR}/peer1
	mkdir -p ${TESTDIR}/peer2
	mkdir -p ${TESTDIR}/peer3

	mkfifo ${PIPE1}
	(cd ${TESTDIR}/peer1 && ../../${PEER} < ../../${PIPE1} &)
	sleep infinity > ${PIPE1}&

	mkfifo ${PIPE2}
	(cd ${TESTDIR}/peer2 && ../../${PEER} < ../../${PIPE2} &)
	sleep infinity > ${PIPE2}&

	mkfifo ${PIPE3}
	(cd ${TESTDIR}/peer3 && ../../${PEER} < ../../${PIPE3} &)
	sleep infinity > ${PIPE3}&

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
	send_input ${PIPE1} "peer1"
	send_input ${PIPE1} "content"

	sleep 1

	send_input ${PIPE2} "2"
	send_input ${PIPE2} "peer2"
	send_input ${PIPE2} "content"

	sleep 1

	send_input ${PIPE3} "2"
	send_input ${PIPE3} "peer3"
	send_input ${PIPE3} "content"

	sleep 1

	send_input ${PIPE1} "3"

	send_input ${PIPE1} "5"
	send_input ${PIPE2} "5"
	send_input ${PIPE3} "5"
}


start_server
peer_create
p2p_test
stop_peers
stop_server
