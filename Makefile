##
# Terminal Picture Render
#
# @file
# @version 0.1

COMPILER=g++
CLANG_TIDY=clang-tidy
BEAR=bear

LINKER_FLAGS=
MAGICK_FLAGS=`Magick++-config --cppflags --cxxflags --ldflags --libs`

FLAGS=-std=c++23 -Wall -Wextra -Werror=switch -Wfatal-errors
DEBUG_FLAGS=-ggdb -O0 -fsanitize=address
RELEASE_FLAGS=-O3 -march=native -fno-trapping-math -fno-math-errno -flto -fomit-frame-pointer

SOURCES=main.cpp
OUTPUT=ttypic

dev: build
	echo "Done ~"

debug: build
	./${OUTPUT}

fast: build-fast
	./${OUTPUT}

check:
	${CLANG_TIDY} ${SOURCES}

build: 
	${BEAR} -- ${COMPILER} ${SOURCES} \
	${FLAGS} ${DEBUG_FLAGS} ${LINKER_FLAGS} ${MAGICK_FLAGS} -o ${OUTPUT} \
	&& ./${OUTPUT}

build-fast: fast
	${COMPILER} ${SOURCES} \
	-O3 \
	${FLAGS} ${RELEASE_FLAGS} ${LINKER_FLAGS} ${MAGICK_FLAGS} -o ${OUTPUT} \
	&& ./${OUTPUT}

clean:
	rm ${OUTPUT}

# end
