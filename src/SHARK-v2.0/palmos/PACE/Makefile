all: 
	rm -rf *.o src; mkdir src
	tools/gencorestub  DALEntryNums.h  -4 src
	tools/gencorestub BootEntryNums.h  -8 src
	tools/gencorestub   UIEntryNums.h -12 src

	for x in src/*.s; do arm-palmos-gcc -mthumb-interwork -c $$x; done
