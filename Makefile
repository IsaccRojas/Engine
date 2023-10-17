# graphics environment libraries
GLLIBS = -lglenv
GLLIBS += -lglfw3dll
GLLIBS += -lglew32
GLLIBS += -lopengl32

# execution environment libraries
EXECLIBS = -lexecenv

# all directories of libraries
LIBDIRS = -L./glenv
LIBDIRS += -L./execenv

# source files
SRCS = src/main.cpp
SRCS += src/loop.cpp
SRCS += src/glinit.cpp

all: out

libglenv.a:
	mingw32-make -C glenv

libexecenv.a:
	mingw32-make -C execenv

out: libglenv.a libexecenv.a
	g++ ${SRCS} ${LIBDIRS} ${GLLIBS} ${EXECLIBS} -o out

# windows syntax
clean:
	rd out.exe
	mingw32-make -C glenv clean
	mingw32-make -C execenv clean