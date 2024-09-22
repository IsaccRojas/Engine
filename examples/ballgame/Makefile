CXXFLAGS = -ggdb -Wall -O0

# graphics environment libraries
GLLIBS = -lglfw3dll
GLLIBS += -lglew32
GLLIBS += -lopengl32

# core library
CORELIB = -lcore

# library directory
LIBDIR = -L./../../core

# source files
SRCS = src/main.cpp
SRCS += src/loop.cpp
SRCS += src/coreinit.cpp
SRCS += src/gfxball.cpp
SRCS += src/physball.cpp
SRCS += src/implementations.cpp

all: out

libcore.a:
	mingw32-make -C ../../core

out: libcore.a
	g++ ${CXXFLAGS} ${SRCS} ${LIBDIR} ${CORELIB} ${GLLIBS} -o out

# windows syntax
clean:
	rd out.exe
	mingw32-make -C core clean