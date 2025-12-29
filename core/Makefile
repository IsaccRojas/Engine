CXXFLAGS = -std=c++17 -Wall -O0 -glldb -fsanitize=address,undefined -fno-omit-frame-pointer
# debug flags: -std=c++17 -Wall -O0 -glldb -fsanitize=address,undefined -fno-omit-frame-pointer
# release flags: -std=c++17 -Wall -O3

SRCDIR = ./src

all: libcore.a

commonexcept.o: ${SRCDIR}/commonexcept.cpp
	clang++ ${CXXFLAGS} -c ${SRCDIR}/commonexcept.cpp -o commonexcept.o

util.o: ${SRCDIR}/util.cpp
	clang++ ${CXXFLAGS} -c ${SRCDIR}/util.cpp -o util.o

script.o: ${SRCDIR}/script.cpp
	clang++ ${CXXFLAGS} -c ${SRCDIR}/script.cpp -o script.o

glutil.o: ${SRCDIR}/glutil.cpp
	clang++ ${CXXFLAGS} -c ${SRCDIR}/glutil.cpp -o glutil.o

glenv.o: ${SRCDIR}/glenv.cpp
	clang++ ${CXXFLAGS} -c ${SRCDIR}/glenv.cpp -o glenv.o

text.o: ${SRCDIR}/text.cpp
	clang++ ${CXXFLAGS} -c ${SRCDIR}/text.cpp -o text.o

animation.o: ${SRCDIR}/animation.cpp
	clang++ ${CXXFLAGS} -c ${SRCDIR}/animation.cpp -o animation.o

filter.o: ${SRCDIR}/filter.cpp
	clang++ ${CXXFLAGS} -c ${SRCDIR}/filter.cpp -o filter.o

physspace.o: ${SRCDIR}/physspace.cpp
	clang++ ${CXXFLAGS} -c ${SRCDIR}/physspace.cpp -o physspace.o

entity.o: ${SRCDIR}/entity.cpp
	clang++ ${CXXFLAGS} -c ${SRCDIR}/entity.cpp -o entity.o

glfwinput.o: ${SRCDIR}/glfwinput.cpp
	clang++ ${CXXFLAGS} -c ${SRCDIR}/glfwinput.cpp -o glfwinput.o

glfwstate.o: ${SRCDIR}/glfwstate.cpp
	clang++ ${CXXFLAGS} -c ${SRCDIR}/glfwstate.cpp -o glfwstate.o

libcore.a: commonexcept.o util.o script.o glutil.o glenv.o text.o animation.o filter.o physspace.o entity.o glfwinput.o glfwstate.o
	llvm-ar rcs core.lib *.o

# windows syntax
clean:
	rd *.o