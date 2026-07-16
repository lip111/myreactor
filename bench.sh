#!/bin/bash

for i in {1..10000}; do
    (sleep 0.5;echo "hello") | nc -q 1 127.0.0.1 8080 &
if [ $((i % 100)) -eq 0 ]; then
        echo "round $i done"
    fi
done