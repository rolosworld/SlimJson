#!/bin/bash

echo "Building decode tests"
gcc -ggdb slim_json_decode.c test_decode.c -o test_decode

echo "Building encode tests"
gcc -ggdb slim_json_decode.c slim_json_encode.c test_encode.c -o test_encode
