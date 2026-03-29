CFLAGS = -Wall -Wextra -g
all: program

program: main.o parent.o worker.o jobs.o filters.o image_io.o io_utils.o
	gcc $(CFLAGS) -o $@ $^ -lm

main.o: main.c parent.h jobs.h protocol.h
	gcc $(CFLAGS) -c $<

parent.o: parent.c parent.h io_utils.h jobs.h protocol.h worker.h
	gcc $(CFLAGS) -c $<

worker.o: worker.c worker.h protocol.h io_utils.h image_io.h filters.h
	gcc $(CFLAGS) -c $<

jobs.o: jobs.c jobs.h image_io.h filters.h
	gcc $(CFLAGS) -c $<

filters.o: filters.c filters.h image_io.h protocol.h
	gcc $(CFLAGS) -c $<

image_io.o: image_io.c image_io.h protocol.h
	gcc $(CFLAGS) -c $<

io_utils.o: io_utils.c io_utils.h
	gcc $(CFLAGS) -c $<

clean:
	rm -f *.o program