#!/bin/sh

./bin/uptime_server.py $2 &
./bin/test $@
