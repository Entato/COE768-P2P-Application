#!/bin/bash

BIN="./bin"
SERVER="${BIN}/server"
CLIENT="${BIN}/client"

start_server() {
	echo "Starting server"
	${SERVER} &
	sleep 2
}

stop_server() {
	pgrep -x "server" > /dev/null
	if [ $? -eq 0 ] 
	then
		pkill -x "server"
		sleep 2
	else
		echo "Server not running"
	fi
}

client_test() {
	${CLIENT} 127.0.0.1 &

}


start_server
client_test
stop_server
