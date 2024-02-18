CC = g++
CCFLAGS = -O3 -Wall -std=c++17
INCLUES = -I.

ifdef RELEASE
else
	# CCFLAGS += -DDEBUG -fsanitize=address,undefined
	CCFLAGS += -DDEBUG 
endif

SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)
EXEC = main

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CCFLAGS) $(INCLUDES) -o $(EXEC) $(OBJS)

rust:
	rustc $(EXEC).rs -o mainrs

.cpp.o:
	$(CC) $(CCFLAGS) -c $< -o $@

clean:
	$(RM) *.o $(EXEC)
