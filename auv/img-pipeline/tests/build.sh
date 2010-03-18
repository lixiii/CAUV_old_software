#! /bin/bash

g++ -I /societies/cauv/install/include/ -I /usr/include/opencv/ -I \
../.. -L /societies/cauv/install/lib/ -lcxcore -lcv \
-lhighgui -lboost_thread -lboost_serialization -lspread ../scheduler.cpp \
../../.obj/debug/common.o -lssrcspread img-pipeline-tester.cpp -o \
img-pipeline-tester -Wall -g -DDEBUG

