CXXFLAGS = -ggdb -Wall -O

SRCDIR = src

all: libcore.a

util.o: ${SRCDIR}/util.cpp
	g++ ${CXXFLAGS} -c ${SRCDIR}/util.cpp -o util.o

script.o: ${SRCDIR}/script.cpp
	g++ ${CXXFLAGS} -c ${SRCDIR}/script.cpp -o script.o

glutil.o: ${SRCDIR}/glutil.cpp
	g++ ${CXXFLAGS} -c ${SRCDIR}/glutil.cpp -o glutil.o

glenv.o: ${SRCDIR}/glenv.cpp
	g++ ${CXXFLAGS} -c ${SRCDIR}/glenv.cpp -o glenv.o

animation.o: ${SRCDIR}/animation.cpp
	g++ ${CXXFLAGS} -c ${SRCDIR}/animation.cpp -o animation.o

entity.o: ${SRCDIR}/entity.cpp
	g++ ${CXXFLAGS} -c ${SRCDIR}/entity.cpp -o entity.o

filter.o: ${SRCDIR}/filter.cpp
	g++ ${CXXFLAGS} -c ${SRCDIR}/filter.cpp -o filter.o

physenv.o: ${SRCDIR}/physenv.cpp
	g++ ${CXXFLAGS} -c ${SRCDIR}/physenv.cpp -o physenv.o

object.o: ${SRCDIR}/object.cpp
	g++ ${CXXFLAGS} -c ${SRCDIR}/object.cpp -o object.o

libcore.a: util.o script.o glutil.o glenv.o animation.o entity.o filter.o physenv.o object.o
	ar rcs libcore.a *.o

# windows syntax
clean:
	rd *.o