OBJ:=main.o Message.o
GCC:=g++
FLAGS:=-c -g -Wall -O0
INC:=./inc/
SRC:=./src/

all: Message
Message: $(OBJ)
	$(GCC) $(OBJ) -o Message
	rm -rf *.o
main.o: $(SRC)main.cpp
	$(GCC) -I$(INC) $(FLAGS) $(SRC)main.cpp
Message.o: $(SRC)Message.cpp
	$(GCC) -I$(INC) $(FLAGS) $(SRC)Message.cpp
clean:
	find . -type f | xargs touch
	rm -rf *.o *scv Message
