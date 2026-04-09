#! /bin/sh
rm -f onewm; ${CC:-tcc} -lX11 onewm.c -o onewm
