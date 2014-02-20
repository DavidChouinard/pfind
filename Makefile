#
# Author: David Chouinard
#   Date: Feb 9 2014
#   Desc: Simple Makefile for compiling pfind
#  Usage: make pfind
#

pfind: pfind.c
	gcc pfind.c -o pfind -Wall -std=gnu99

clean:
	rm -f *.o core pfind
