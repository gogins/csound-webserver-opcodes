#!/bin/bash

echo "Generating abstract skeletons and stubs..."
jsonrpcstub --verbose csoundrpc.json --js-client=Csound --cpp-server=CsoundSkeleton

echo "Compiling the WebKit opcodes..."
g++ webkit_opcodes.cpp -g -fPIC -I/usr/local/include -I/usr/local/include/csound -I. `pkg-config gtkmm-3.0 --libs --cflags` `pkg-config webkit2gtk-4.0 --libs --cflags` -std=c++17 -ljsoncpp -lmicrohttpd -ljsonrpccpp-common -ljsonrpccpp-server -shared -owebkit_opcodes.so

echo "Finished."
ls -ll
