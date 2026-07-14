#!/bin/bash

for i in {1..1000}; do
    echo "hello" | nc -q 1 127.0.0.1 8080 &
if [ $((i % 100)) -eq 0 ]; then
        echo "round $i done"
    fi
done