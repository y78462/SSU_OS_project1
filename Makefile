
ssu_shell : ssu_shell.o pps.o
	gcc ssu_shell.o -o ssu_shell
	gcc pps.o -o pps

ssu_shell.o : ssu_shell.c
	gcc -c ssu_shell.c

pps.o : pps.c
	gcc -c pps.c

clean :
	rm *.o
	rm ssu_shell
	rm pps

