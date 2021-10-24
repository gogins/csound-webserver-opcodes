#!/bin/bash

echo "Generating abstract skeletons and stubs from csoundrpc.json..."
jsonrpcstub --verbose csoundrpc.json --js-client=Csound --cpp-server=CsoundSkeleton

echo "Compiling the WebKit opcodes..."
g++ webkit_opcodes.cpp -std=c++2a -g -fPIC -I/home/mkg/uWebSockets/src -I/home/mkg/uWebSockets/uSockets/src -I/usr/local/include -I/usr/local/include/csound -I. `pkg-config gtkmm-3.0 --libs --cflags` `pkg-config webkit2gtk-4.0 --libs --cflags` -ljsoncpp -lmicrohttpd -ljsonrpccpp-common -ljsonrpccpp-server /home/mkg/uWebSockets/uSockets/uSockets.a -shared -owebkit_opcodes.so

echo "Finished."
ls -ll
