#!/bin/sh

NODE_COUNT=10
THRESHOLD_FRACTION=0.7
TRIAL_SIZE=4096

export NODE_COUNT
export THRESHOLD_FRACTION
export TRIAL_SIZE

if [ "$1" = "--pull" ]; then
	docker pull gabrielkulp/dise
	docker pull gabrielkulp/htdise
fi

# Until this gets addressed, I have to use some ugly hacks to get only the output
# https://github.com/docker/compose/issues/6026
if [ "$1" = "--build" ]; then
	docker-compose up --build
else
	docker-compose up 2>/dev/null | grep "enc/s:" | cut -d\| -f 2 | sed 's/^[[:space:]]*//'
fi

docker run --rm -it gabrielkulp/dise /usr/local/bin/dEncFrontend \
	-ss -nStart "${NODE_COUNT}" -mf "${THRESHOLD_FRACTION}" -t "${TRIAL_SIZE}" -l
