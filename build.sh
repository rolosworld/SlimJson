#!/bin/bash

gcc -ggdb slim_json_decode.c test_decode.c -o test_decode
gcc -ggdb slim_json_encode.c test_encode.c -o test_encode
