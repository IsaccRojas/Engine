SRCDIR = src

all: libexecenv.a

util.o: ${SRCDIR}/util.cpp
	g++ -c ${SRCDIR}/util.cpp -o util.o

script.o: ${SRCDIR}/script.cpp
	g++ -c ${SRCDIR}/script.cpp -o script.o

execenv.o: ${SRCDIR}/execenv.cpp
	g++ -c ${SRCDIR}/execenv.cpp -o execenv.o

libexecenv.a: util.o script.o execenv.o
	ar rcs libexecenv.a util.o script.o execenv.o

# windows syntax
clean:
	rd libexecenv.a
	rd execenv.o