CXXFLAGS = -std=c++17 -Wall -O0 -glldb -fsanitize=address,undefined -fno-omit-frame-pointer
# debug flags: -std=c++17 -Wall -O0 -glldb -fsanitize=address,undefined -fno-omit-frame-pointer
# release flags: -std=c++17 -Wall -O3

# graphics environment libraries
GLLIBS = -llibglfw3dll.lib
GLLIBS += -lglew32.lib
GLLIBS += -lopengl32.lib

# core library
CORELIB = -lcore.lib

# library directory
LIBDIR = -L./../../core -LC:/dev/lib

# source files
SRCS = src/main.cpp
SRCS += src/loop.cpp
SRCS += src/coreinit.cpp
SRCS += src/gfxball.cpp
SRCS += src/physball.cpp
SRCS += src/implementations.cpp

all: out

libcore.a:
	make -C ../../core

out: libcore.a
	clang++ ${CXXFLAGS} ${SRCS} ${LIBDIR} ${CORELIB} ${GLLIBS} -o out.exe

# windows syntax
clean:
	rd out.exe
	make -C core clean