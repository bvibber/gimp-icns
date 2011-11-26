LIBS = `gimptool-2.0 --libs`
CFLAGS = `gimptool-2.0 --cflags` -I/sw/include

icns : main.o icnsload.o icnsdata.o icnssave.o
	gcc -o icns main.o icnsload.o icnsdata.o icnssave.o $(LIBS)

main.o : main.c icnsdata.h main.h
	gcc $(CFLAGS) -c main.c -o main.o

icnsload.o : icnsload.c icnsload.h icnsdata.h main.h
	gcc $(CFLAGS) -c icnsload.c -o icnsload.o

icnssave.o : icnssave.c icnssave.h icnsdata.h main.h
	gcc $(CFLAGS) -c icnssave.c -o icnssave.o

icnsdata.o : icnsdata.c icnsdata.h
	gcc $(CFLAGS) -c icnsdata.c -o icnsdata.o

icnsload.h :

icnssave.h :

main.h :

icnsdata.h :

clean :
	rm -f *.o icns

install : icns
	gimptool-2.0 --install-bin icns

install-local : icns
	cp icns $(HOME)/.gimp-2.6/plug-ins/

