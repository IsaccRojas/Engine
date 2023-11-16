CXXFLAGS = -ggdb

# graphics environment libraries
GLLIBS = -lglenv
GLLIBS += -lglfw3dll
GLLIBS += -lglew32
GLLIBS += -lopengl32

# execution environment libraries
EXECLIBS = -lexecutor

# all directories of libraries
LIBDIRS = -L./glenv
LIBDIRS += -L./executor

# source files
SRCS = src/main.cpp
SRCS += src/loop.cpp
SRCS += src/glinit.cpp
SRCS += src/entity.cpp
SRCS += src/animation.cpp
SRCS += src/object.cpp

all: out

libglenv.a:
	mingw32-make -C glenv

libexecutor.a:
	mingw32-make -C executor

out: libglenv.a libexecutor.a
	g++ ${CXXFLAGS} ${SRCS} ${LIBDIRS} ${GLLIBS} ${EXECLIBS} -o out

# windows syntax
clean:
	rd out.exe
	mingw32-make -C glenv clean
	mingw32-make -C executor clean