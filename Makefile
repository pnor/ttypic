##
# ttypic
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
INCLUDE_DIR=./include

SOURCES=main.cpp
OUTPUT=ttypic
TEST_IMAGE=./test_image.jpg

dev: debug
	echo "Done"

debug: build
	./${OUTPUT} ${TEST_IMAGE}

fast: build-fast
	./${OUTPUT} ${TEST_IMAGE}

check:
	${CLANG_TIDY} ${SOURCES}

build: 
	${BEAR} -- ${COMPILER} -I${INCLUDE_DIR} ${SOURCES} \
	${FLAGS} ${DEBUG_FLAGS} ${LINKER_FLAGS} ${MAGICK_FLAGS} -o ${OUTPUT}

build-fast: fast
	${COMPILER} -I${INCLUDE_DIR} ${SOURCES} \
	-O3 \
	${FLAGS} ${RELEASE_FLAGS} ${LINKER_FLAGS} ${MAGICK_FLAGS} -o ${OUTPUT}

clean:
	rm ${OUTPUT}

# end
