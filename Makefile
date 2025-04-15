CC = g++

CompileParms = -c -Wall -std=c++11 -O2

OBJS = kern.o standaard.o woordsompuzzel.o main.o

Opdr: $(OBJS)
	$(CC) $(OBJS) -o WoordSomPuzzel

clean:
	rm -f *.o WoordSomPuzzel

kern.o: kern.c kern.h
	$(CC) $(CompileParms)  kern.c

standaard.o: standaard.cc standaard.h
	$(CC) $(CompileParms)  standaard.cc

woordsompuzzel.o: woordsompuzzel.cc kern.h constantes.h woordsompuzzel.h
	$(CC) $(CompileParms)  woordsompuzzel.cc

main.o: main.cc kern.h constantes.h woordsompuzzel.h
	$(CC) $(CompileParms)  main.cc

