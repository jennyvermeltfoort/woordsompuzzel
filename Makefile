CC = g++

CompileParms = -c -Wall -std=c++11 -O2

OBJS = standaard.o woordsompuzzel.o main.o

Opdr: $(OBJS)
	$(CC) $(OBJS) -o WoordSomPuzzel

clean:
	rm -f *.o WoordSomPuzzel

standaard.o: standaard.cc standaard.h
	$(CC) $(CompileParms)  standaard.cc

woordsompuzzel.o: woordsompuzzel.cc constantes.h woordsompuzzel.h
	$(CC) $(CompileParms)  woordsompuzzel.cc

main.o: main.cc constantes.h woordsompuzzel.h
	$(CC) $(CompileParms)  main.cc

