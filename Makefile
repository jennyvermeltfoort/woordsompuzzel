CC = g++

CompileParms = -c -g -Wall -std=c++11 

OBJS =  standaard.o woordsompuzzel.o main.o pman.o

Opdr: $(OBJS)
	$(CC) $(OBJS) -o WoordSomPuzzel

clean:
	rm -f *.o WoordSomPuzzel

pman.o: pman.c pman.h
	gcc --std=c99 -g -c -Wall  pman.c

standaard.o: standaard.cc standaard.h
	$(CC) $(CompileParms) standaard.cc

woordsompuzzel.o: woordsompuzzel.cc pman.h constantes.h woordsompuzzel.h
	$(CC) $(CompileParms) woordsompuzzel.cc

main.o: main.cc pman.h constantes.h woordsompuzzel.h
	$(CC) $(CompileParms) main.cc