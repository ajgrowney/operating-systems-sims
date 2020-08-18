cachesim: cachesim.c
	gcc -o cachesim cachesim.c -I.

clean:
	rm cachesim

push:
	git stash
	git pull
	git stash pop
	make clean
	git add .
	git commit -m "testing new makefile command"
	git push

test_PCANNY_1:
	make clean
	make
	./cachesim ~/Downloads/AddressTrace_FirstIndex.bin

test_PCANNY_2:
	make clean
	make
	./cachesim ~/Downloads/AddressTrace_LastIndex.bin

test_PCANNY_3:
	make clean
	make
	./cachesim ~/Downloads/AddressTrace_RandomIndex.bin
