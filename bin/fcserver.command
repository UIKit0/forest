#!/bin/sh
DIR=`dirname "$0"`
while true; do
    "$DIR/fcserver-osx" "$DIR/fcserver.json"
    sleep 2
done

