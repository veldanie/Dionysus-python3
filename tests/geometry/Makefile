SYNAPS_LIBS = -L/usr/lib -L/usr/lib -lsynaps -llapack -lblas -lgmpxx -lgmp -lmps -L/usr/lib/gcc/i686-pc-linux-gnu/4.1.2 -L/usr/lib/gcc/i686-pc-linux-gnu/4.1.2/../../.. -lgfortranbegin -lgfortran -lm -lgcc_s

INCLUDE_PATH=-I../../include/geometry
DEFINES=-DBOOST_UBLAS_TYPE_CHECK=0 -g
LIBRARIES=-lsynaps -lgmp -lgmpxx #${SYNAPS_LIBS}
#CPPFLAGS=-march=i686 -mtune=native -Wall
#CPPFLAGS=-march=i686 -mtune=generic -Wall
CPPFLAGS=-Wall
GCC=g++

all: test-linalg euclidean polynomial test-eventqueue test-kinetic-sort

test-linalg: test-linalg.cpp
	${GCC} test-linalg.cpp -o test-linalg ${INCLUDE_PATH} ${DEFINES} ${LIBRARIES} ${CPPFLAGS}

euclidean: euclidean.cpp
	${GCC} euclidean.cpp -o euclidean ${INCLUDE_PATH} ${DEFINES} ${CPPFLAGS}

polynomial: polynomial.cpp
	${GCC} polynomial.cpp -o polynomial ${INCLUDE_PATH} ${DEFINES} ${LIBRARIES} ${CPPFLAGS}

test-eventqueue: test-eventqueue.cpp
	${GCC} test-eventqueue.cpp -o test-eventqueue ${CPPFLAGS} ${DEFINES} ${INCLUDE_PATH}

test-kinetic-sort: test-kinetic-sort.cpp
	${GCC} test-kinetic-sort.cpp -o test-kinetic-sort ${INCLUDE_PATH} ${DEFINES} ${LIBRARIES} ${CPPFLAGS}
