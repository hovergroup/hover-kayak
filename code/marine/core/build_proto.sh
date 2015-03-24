#!/bin/bash

protoc --proto_path=src/protobuf --cpp_out=src/protobuf --proto_path=$HOME/hoverland/goby/include --proto_path=/usr/include src/protobuf/*.proto
