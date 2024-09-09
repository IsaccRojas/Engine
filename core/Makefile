CXXFLAGS = -ggdb -Wall -O0

SRCDIR = src

all: libcore.a

commonexcept.o: ${SRCDIR}/commonexcept.cpp
	g++ ${CXXFLAGS} -c ${SRCDIR}/commonexcept.cpp -o commonexcept.o

util.o: ${SRCDIR}/util.cpp
	g++ ${CXXFLAGS} -c ${SRCDIR}/util.cpp -o util.o

script.o: ${SRCDIR}/script.cpp
	g++ ${CXXFLAGS} -c ${SRCDIR}/script.cpp -o script.o

glutil.o: ${SRCDIR}/glutil.cpp
	g++ ${CXXFLAGS} -c ${SRCDIR}/glutil.cpp -o glutil.o

glenv.o: ${SRCDIR}/glenv.cpp
	g++ ${CXXFLAGS} -c ${SRCDIR}/glenv.cpp -o glenv.o

text.o: ${SRCDIR}/text.cpp
	g++ ${CXXFLAGS} -c ${SRCDIR}/text.cpp -o text.o

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

input.o: ${SRCDIR}/input.cpp
	g++ ${CXXFLAGS} -c ${SRCDIR}/input.cpp -o input.o

libcore.a: commonexcept.o util.o script.o glutil.o glenv.o text.o animation.o entity.o filter.o physenv.o object.o input.o
	ar rcs libcore.a *.o

# windows syntax
clean:
	rd *.o