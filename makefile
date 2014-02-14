#-----------------------------------------------------------------------
# Use the GNU C/C++ compiler:
CC = gcc
CPP = g++

# COMPILER OPTIONS:

CFLAGS = -c

#OBJECT FILES
OBJS = prog1.o

dsh: prog1.o
	${CPP} -lm ${OBJS} -o dsh
prog1.o: prog1.cpp

#-----------------------------------------------------------------------
