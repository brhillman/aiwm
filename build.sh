#! /bin/sh

rm -f aiwm; ${CC:-tcc} -lX11 aiwm.c -o aiwm
