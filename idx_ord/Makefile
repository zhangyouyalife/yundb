CFLAGS=-I.

defaut: createdb main

clean:
	rm createdb main *.o block/*.o block/*.a

createdb: createdb.o db.o datadict.o file.o file_heap.o block/block.a tuple.o data.o buf.o sysfile.o list.o

main: main.o db.o datadict.o file.o file_heap.o block/block.a tuple.o data.o buf.o sysfile.o list.o

data.o: data.c data.h

tuple.o: tuple.c tuple.h

block/block.o: block/block.c block/block.h

block/block_heap.o: block/block_heap.c block/block_heap.h

block/block_seq.o: block/block_seq.c block/block_seq.h

block/block.a: block/block.o block/block_heap.o block/block_seq.o
	ar rcs block/block.a block/block.o block/block_heap.o block/block_seq.o

file.o: file.c file.h conf.h

file_heap.o: file_heap.c file_heap.h

file_order.o: file_order.c file_order.h

db.o: db.c db.h datadict.h

datadict.o: datadict.c datadict.h

buf.o: buf.c buf.h

sysfile.o: sysfile.c sysfile.h

list.o: list.c list.h

createdb.o: createdb.c datadict.h

