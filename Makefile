all: NavicellaClient MeteoritiServer
NavicellaClient: client_navicella.o
	gcc -o NavicellaClient client_navicella.o
MeteoritiServer: server_meteoriti.o
	gcc -o MeteoritiServer server_meteoriti.o
client_navicella.o: client_navicella.c
	gcc -c client_navicella.c
server_meteoriti.o: server_meteoriti.c
	gcc -c server_meteoriti.c
clean:
	rm -f *.o