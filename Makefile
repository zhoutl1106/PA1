pa1 : main.o zfs.o mkfs_net.o
	g++ -o pa1 main.o zfs.o mkfs_net.o

main.o : main.cpp
	g++ -c main.cpp
zfs.o : zfs.cpp zfs.h
	g++ -c zfs.cpp

mkfs_net : mkfs_net.h
	g++ -c mkfs.net.cpp

clean:
	rm *.o
