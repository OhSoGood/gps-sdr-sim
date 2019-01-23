# Makefile for Linux etc.

.PHONY: all clean time
all: gps-sdr-sim

SHELL=/bin/bash
CC=gcc
CFLAGS=-O3 -Wall -D_FILE_OFFSET_BITS=64
CFLAGS+=-DUSER_MOTION_SIZE=12000
LDFLAGS=-lm

gps-sdr-sim: gpssim.o
	${CC} $< ${LDFLAGS} -o $@

gpssim.o: .user-motion-size

.user-motion-size: .FORCE
	@if [ -f .user-motion-size ]; then \
		if [ "`cat .user-motion-size`" != "$(USER_MOTION_SIZE)" ]; then \
			echo "Updating .user-motion-size"; \
			echo "$(USER_MOTION_SIZE)" >| .user-motion-size; \
		fi; \
	else \
		echo "$(USER_MOTION_SIZE)" > .user-motion-size; \
	fi;

clean:
	rm -f gpssim.o gps-sdr-sim *.bin .user-motion-size

time: gps-sdr-sim
	time ./gps-sdr-sim -e brdc3540.14n -u circle.csv -b 1
	time ./gps-sdr-sim -e brdc3540.14n -u circle.csv -b 8
	time ./gps-sdr-sim -e brdc3540.14n -u circle.csv -b 16

meacon.elf: .FORCE
	g++ -O3 -std=c++17 meacon.cc -o meacon.elf

advance.elf: .FORCE
	g++ -O3 -std=c++17 advance.cc -o advance.elf

doe_demo.elf: .FORCE
	g++ -O3 -std=c++17 doe_demo.cc -o doe_demo.elf

gpssim.bin: gps-sdr-sim
	./gps-sdr-sim -e brdc0010.19n -l 30.648158,-96.475147

demo_gpssim.bin: doe_demo.elf gpssim.bin
	./doe_demo.elf gpssim.bin

.FORCE:

YEAR?=$(shell date +"%Y")
Y=$(patsubst 20%,%,$(YEAR))
%.$(Y)n:
	wget -q ftp://cddis.gsfc.nasa.gov/gnss/data/daily/$(YEAR)/brdc/$@.Z -O $@.Z
	uncompress $@.Z
