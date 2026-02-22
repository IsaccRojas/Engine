CXXFLAGS = -std=c++17 -Wall -O0 -glldb -fsanitize=address,undefined -fno-omit-frame-pointer
# debug flags: -std=c++17 -Wall -O0 -glldb -fsanitize=address,undefined -fno-omit-frame-pointer
# release flags: -std=c++17 -Wall -O3

# graphics environment libraries
GLLIBS = -llibglfw3dll
GLLIBS += -lglew32
GLLIBS += -lopengl32

# core library
CORELIB = -lcore

# library directory
LIBDIR = -L./../../core -LC:/dev/lib

# source files
SRCS = src/main.cpp
SRCS += src/coreinit.cpp
SRCS += src/loop.cpp
#SRCS += src/effect.cpp
#SRCS += src/object.cpp
SRCS += src/implementations.cpp

all: out

libcore.a:
	mingw32-make -C ../../core

out: libcore.a
	clang++ ${CXXFLAGS} ${SRCS} ${LIBDIR} ${CORELIB} ${GLLIBS} -o out.exe

# windows syntax
clean:
	rd out.exe
	make -C core clean