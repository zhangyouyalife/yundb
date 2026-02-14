defaut: createdb main

clean:
	rm createdb main *.o

createdb: createdb.o db.o datadict.o file.o file_heap.o blk.o tuple.o data.o buf.o sysfile.o list.o

main: main.o db.o datadict.o file.o file_heap.o blk.o tuple.o data.o buf.o sysfile.o list.o

data.o: data.c data.h

tuple.o: tuple.c tuple.h

blk.o: blk.c blk.h

file.o: file.c file.h conf.h

file_heap.o: file_heap.c file_heap.h

db.o: db.c db.h datadict.h

datadict.o: datadict.c datadict.h

buf.o: buf.c buf.h

sysfile.o: sysfile.c sysfile.h

list.o: list.c list.h

createdb.o: createdb.c datadict.h

