CXXFLAGS = -ggdb

SRCDIR = src

all: libglenv.a

util.o: ${SRCDIR}/util.cpp
	g++ ${CXXFLAGS} -c ${SRCDIR}/util.cpp -o util.o

glutil.o: ${SRCDIR}/glutil.cpp
	g++ ${CXXFLAGS} -c ${SRCDIR}/glutil.cpp -o glutil.o

glenv.o: ${SRCDIR}/glenv.cpp
	g++ ${CXXFLAGS} -c ${SRCDIR}/glenv.cpp -o glenv.o

libglenv.a: util.o glutil.o glenv.o
	ar rcs libglenv.a util.o glutil.o glenv.o

# windows syntax
clean:
	rd libglenv.a
	rd glenv.o
	rd glutil.o
	rd util.o