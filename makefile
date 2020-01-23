

p2:  p2.c
	cc -Ofast -o p2  p2.c -lm -lpthread

clean:
	rm -f p2 *.o
