#!/bin/bash

NODE_COUNT=$1
M=$2
reviewers_protocol=false
THRESHOLD_FRACTION=`echo "(($M / $NODE_COUNT))" | bc -l`
TRIAL_SIZE=100000

SERVER_COUNT=$((NODE_COUNT-1)) # one is the client
export SERVER_COUNT
export THRESHOLD_FRACTION
export TRIAL_SIZE

if [ "$3" = "--pull" ]; then
	docker pull gabrielkulp/dise
	docker pull gabrielkulp/htdise
fi

echo "" > schedule.log

if [ "$3" = "--build" ]; then
	docker-compose down
	sleep 5
	if [ "${M}" = 2 ] && [ "$reviewers_protocol" = true ]; then
		cp data/reboot_sequence_logs/states_"${NODE_COUNT}"_1_100k.txt data/state.txt
	else
		cp data/reboot_sequence_logs/states_"${NODE_COUNT}"_"${M}"_1M.txt data/state.txt
	fi
	docker build . -t groundhog:latest || exit 1
	sleep 5
	docker-compose up --remove-orphans || exit 2 # show full log
else
# Until this gets addressed, I have to use some ugly hacks to get only the output
# https://github.com/docker/compose/issues/6026
	docker-compose up 2>/dev/null | grep "enc/s:" | cut -d\| -f 2 | sed 's/^[[:space:]]*//'
fi
exit 1
#docker run --privileged --rm -it --security-opt seccomp=/usr/local/default.json groundhog:latest /usr/local/bin/dEncFrontend 
#-ss -nStart "${NODE_COUNT}" -mf "${THRESHOLD_FRACTION}" -t "${TRIAL_SIZE}" -l
