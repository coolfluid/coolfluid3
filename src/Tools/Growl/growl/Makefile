all: dirs bin/gntp-send bin/gntp-send++

bin/gntp-send : objs/gntp-send.o lib/libgrowl.a
	gcc $^ -o $@

bin/gntp-send++ : objs/gntp-send++.o lib/libgrowl++.a lib/libgrowl.a
	g++ $^ -o $@

lib/libgrowl.a : objs/growl.o objs/tcp.o  objs/md5.o
	ar rc $@ $^
	ranlib $@	

lib/libgrowl++.a : objs/growl++.o
	ar rc $@ $^
	ranlib $@

objs/growl.o : source/growl.c
	gcc -D GROWL_STATIC -I headers -Wall -Wno-format-zero-length -c $< -o $@

objs/tcp.o : source/tcp.c
	gcc -I headers -Wall -c $< -o $@

objs/md5.o : source/md5.c
	gcc -I headers -Wall -c $< -o $@

objs/gntp-send.o : source/gntp-send.c
	gcc -I headers -Wall -c $< -o $@

objs/growl++.o : source/growl++.cpp
	g++ -I headers -Wall -c $< -o $@

objs/gntp-send++.o : source/gntp-send++.cpp
	g++ -I headers -Wall -c $< -o $@

clean : 
	rm -f bin/* objs/* lib/*

dirs : bin objs lib
bin :
	mkdir $@
objs :
	mkdir $@
lib :
	mkdir $@
