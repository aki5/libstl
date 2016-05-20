
CFLAGS=-O3 -fomit-frame-pointer

HFILES=\
	stlfile.h\
	hash64.h\
	hash96.h\

OFILES=\
	stlfile.o\
	halfedge.o\

libstl.a: $(OFILES)
	$(AR) r $@ $(OFILES)

stltest: stltest.o libstl.a
	$(CC) -o $@ stltest.o libstl.a

triple_gear.stl:
	curl -L -o triple_gear.stl http://www.thingiverse.com/download:182636

test: stltest triple_gear.stl
	./stltest -n5 triple_gear.stl

clean:
	rm -f stltest libstl.a *.o

distclean: clean
	rm -f triple_gear.stl

