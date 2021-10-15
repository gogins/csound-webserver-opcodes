#!/bin/bash
g++ webkit_opcodes.cpp -I/usr/local/include -I/usr/local/include/csound -I. `pkg-config gtkmm-3.0 --libs --cflags` `pkg-config webkit2gtk-4.0 --libs --cflags` -std=c++17 -lcsound64 -otest

#g++ -Dlinux --std=gnu++17 -lstdc++fs -O3 -g -Wno-write-strings -I.  -I/usr/local/include  "$(FilePath)" -fPIC -shared -olib$(FileName).so -I/usr/local/include/csound -I/usr/include/csound -I/usr/include/luajit-2.1 -lGamma -lCsoundAC -lluajit-5.1 -lsndfile -lgc -lpthread -ldl -lm

g++ webkit_opcodes.cpp -g -fPIC -I/usr/local/include -I/usr/local/include/csound -I. `pkg-config gtkmm-3.0 --libs --cflags` `pkg-config webkit2gtk-4.0 --libs --cflags` -std=c++17 -shared -owebkit_opcodes.so

ls -ll

### ./test
