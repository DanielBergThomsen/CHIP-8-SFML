#OBJS specifies which files to compile as part of the project
OBJS = Main.cpp Chip8.cpp Chip8.h

#CC specifies which compiler we're using
CC = g++

#COMPILER_FLAGS specifies the additional compilation options we're using
# -w suppresses all warnings
COMPILER_FLAGS = -w -g -std=c++11

#LINKER_FLAGS specifies the libraries we're linking against
LINKER_FLAGS = -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = chip-8

#This is the target that compiles our executable
all : $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
