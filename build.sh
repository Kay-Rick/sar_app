#!/usr/bin/bash
protoc sar_application.proto --cpp_out=./ --proto_path=./
srpc_generator protobuf ./sar_application.proto ./
rm server.pb_skeleton.cc client.pb_skeleton.cc

make

