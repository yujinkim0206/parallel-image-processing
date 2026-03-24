CFLAGS = -Wall -Wextra -g
all: program

program: main.o worker.o ppm.o filters.o io_utils.o
	gcc $(CFLAGS) -o $@ $^ -lm

main.o: main.c protocol.h ppm.h filters.h worker.h io_utils.h
	gcc $(CFLAGS) -c $<

worker.o: worker.c protocol.h ppm.h filters.h worker.h io_utils.h
	gcc $(CFLAGS) -c $<

ppm.o: ppm.c ppm.h 
	gcc $(CFLAGS) -c $<

filters.o: filters.c filters.h ppm.h 
	gcc $(CFLAGS) -c $<

io_utils.o: io_utils.c io_utils.h protocol.h
	gcc $(CFLAGS) -c $<

clean:
	rm -f *.o program