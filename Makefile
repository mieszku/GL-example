##
 # Copyright (c) 2016 Mieszko Mazurek
 ##

build_dir = /tmp/gl
source	= renderer.cc main.cc display_x11.cc cube.cc
objects	= ${source:%=${build_dir}/%.o}
deps	= ${objects:%.o=%.dep}
target	= program

cc	= gcc -c
cxx	= g++ -c
dc	= dmd -c
ld	= g++

cflags	=
cxxflags = -std=gnu++14 -D__USE_LOG -Wall -MMD -Og -g
dflags	= 
ldflags	= -lGL -lX11 -lGLU

all: ${target}

run: ${target}
	./$<

clean:
	rm -rf ${build_dir}

${target}: ${objects}
	${ld} $^ -o $@ ${ldflags}

${build_dir}/%.c.o: %.c
	mkdir -p ${dir $@}
	${cc} $< -o $@ ${cflags}

${build_dir}/%.d.o: %.d
	mkdir -p ${dir $@}
	${dc} $< -of$@ ${dflags}

${build_dir}/%.cc.o: %.cc
	mkdir -p ${dir $@}
	${cxx} $< -o $@ ${cxxflags}

-include ${deps}
