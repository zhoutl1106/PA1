pa1 : main.o zfs.o
	g++ -o pa1 main.o zfs.o

main.o : main.cpp
	g++ -c main.cpp
zfs.o : zfs.cpp zfs.h
	g++ -c zfs.cpp


clean:
	rm *.o
