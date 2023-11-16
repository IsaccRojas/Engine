CXXFLAGS = -ggdb

SRCDIR = src

all: libexecutor.a

util.o: ${SRCDIR}/util.cpp
	g++ ${CXXFLAGS} -c ${SRCDIR}/util.cpp -o util.o

executor.o: ${SRCDIR}/executor.cpp
	g++ ${CXXFLAGS} -c ${SRCDIR}/executor.cpp -o executor.o

libexecutor.a: util.o executor.o
	ar rcs libexecutor.a util.o executor.o

# windows syntax
clean:
	rd libexecutor.a
	rd executor.o