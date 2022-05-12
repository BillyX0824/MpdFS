OPTIMIZE := -O3
WARNINGS := -Wall
EXTRA_CFLAGS := -std=c++0x -DDEBUG=0 -D_FILE_OFFSET_BITS=64

############################################

CURRENT = $(CURDIR)
SRC=				${CURRENT}/src
CHUNKSERVER_DIR=			${SRC}/chunkserver
MONITOR_DIR=		${SRC}/monitor
CLIENT_DIR=			${SRC}/client
FUSE_DIR=			${SRC}/fuse
MASTER_DIR=			${SRC}/master
COMMON_DIR=			${SRC}/common
PROTOCOL_DIR=		${SRC}/protocol
BENCHMARK_DIR=		${SRC}/benchmark
CODING_TESTER_DIR=		${SRC}/coding_tester


default:
	make master;
	make client;
	make chunkserver;
	make monitor;
	make clientfuse;

all:
	make master;
	make client;
	make chunkserver;
	make monitor;
	make clientfuse;
	make benchmark;

master:
	cd ${MASTER_DIR}; make OPTIMIZE="$(OPTIMIZE)" WARNINGS="$(WARNINGS)" EXTRA_CFLAGS="$(EXTRA_CFLAGS)" -j3

client:
	cd ${CLIENT_DIR}; make OPTIMIZE="$(OPTIMIZE)" WARNINGS="$(WARNINGS)" EXTRA_CFLAGS="$(EXTRA_CFLAGS)" -j3

chunkserver:
	cd ${CHUNKSERVER_DIR}; make OPTIMIZE="$(OPTIMIZE)" WARNINGS="$(WARNINGS)" EXTRA_CFLAGS="$(EXTRA_CFLAGS)" -j3

monitor:
	cd ${MONITOR_DIR}; make OPTIMIZE="$(OPTIMIZE)" WARNINGS="$(WARNINGS)" EXTRA_CFLAGS="$(EXTRA_CFLAGS)" -j3

codingtester:
	cd ${CODING_TESTER_DIR}; make OPTIMIZE="$(OPTIMIZE)" WARNINGS="$(WARNINGS)" EXTRA_CFLAGS="$(EXTRA_CFLAGS)" -j3

clientfuse:
	cd ${FUSE_DIR}; make OPTIMIZE="$(OPTIMIZE)" WARNINGS="$(WARNINGS)" EXTRA_CFLAGS="$(EXTRA_CFLAGS)" -j3

enum:
	cd ${COMMON_DIR}; make

protocol:
	cd ${PROTOCOL_DIR}; make

benchmark:
	cd ${BENCHMARK_DIR}; make OPTIMIZE="$(OPTIMIZE)" WARNINGS="$(WARNINGS)" EXTRA_CFLAGS="$(EXTRA_CFLAGS)" -j3

clean:
	cd ${CHUNKSERVER_DIR}; make clean
	cd ${MONITOR_DIR}; make clean
	cd ${CLIENT_DIR}; make clean
	cd $(FUSE_DIR); make clean
	cd ${MASTER_DIR}; make clean
	cd ${CODING_TESTER_DIR}; make clean
	cd ${BENCHMARK_DIR}; make clean
