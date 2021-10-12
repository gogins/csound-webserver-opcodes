#!/bin/bash
g++ webkit_opcodes.cpp -I/usr/local/include -I/usr/local/include/csound -I. `pkg-config gtkmm-3.0 --libs --cflags` `pkg-config webkit2gtk-4.0 --libs --cflags` -std=c++17 -lcsound64 -otest

g++ webkit_opcodes.cpp -fPIC -I/usr/local/include -I/usr/local/include/csound -I. `pkg-config gtkmm-3.0 --libs --cflags` `pkg-config webkit2gtk-4.0 --libs --cflags` -std=c++17 -shared -owebkit_opcodes.so

ls -ll

./test
