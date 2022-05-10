#!/bin/sh

NODE_COUNT=10
THRESHOLD_FRACTION=0.5
TRIAL_SIZE=409600

SERVER_COUNT=$((NODE_COUNT-1)) # one is the client
export SERVER_COUNT
export THRESHOLD_FRACTION
export TRIAL_SIZE

if [ "$1" = "--pull" ]; then
	docker pull gabrielkulp/dise
	docker pull gabrielkulp/htdise
fi

echo "" > schedule.log

if [ "$1" = "--build" ]; then
	docker-compose down
	sleep 1
	docker build . -t gabrielkulp/htdise:latest || exit 1
	sleep 8
	docker-compose up --remove-orphans || exit 2 # show full log
else
	# Until this gets addressed, I have to use some ugly hacks to get only the output
	# https://github.com/docker/compose/issues/6026
	docker-compose up 2>/dev/null | grep "enc/s:" | cut -d\| -f 2 | sed 's/^[[:space:]]*//'
fi

exit 1
docker run --rm -it gabrielkulp/dise /usr/local/bin/dEncFrontend \
	-ss -nStart "${NODE_COUNT}" -mf "${THRESHOLD_FRACTION}" -t "${TRIAL_SIZE}" -l
