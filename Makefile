CFLAGS = -Wall -Wextra -g
all: program

program: main.o worker.o ppm.o filters.o
	gcc $(CFLAGS) -o $@ $^ -lm

main.o: main.c common.h ppm.h filters.h worker.h
	gcc $(CFLAGS) -c $<

worker.o: worker.c common.h ppm.h filters.h worker.h
	gcc $(CFLAGS) -c $<

ppm.o: ppm.c ppm.h common.h
	gcc $(CFLAGS) -c $<

filters.o: filters.c filters.h ppm.h common.h
	gcc $(CFLAGS) -c $<

clean:
	rm -f *.o program