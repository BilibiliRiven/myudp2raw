udp2raw: main.cpp share.cpp share.h
	g++ main.cpp  share.cpp -o udp2raw -I include

clean: 
	rm -rf *.o *.s
	rm udp2raw