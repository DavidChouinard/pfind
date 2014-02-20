#
# Author: David Chouinard
#   Date: Feb 9 2014
#   Desc: Simple Makefile for compiling pfind
#  Usage: make pfind
#

pfind: pfind.c
	c99 pfind.c -o pfind -Wall

clean:
	rm -f *.o core pfind
