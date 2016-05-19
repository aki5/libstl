
OFILES=\
	stlfile.o\

libstl.a: $(OFILES)
	$(AR) r $@ $(OFILES)

stltest: stltest.o libstl.a
	$(CC) -o $@ stltest.o libstl.a

triple_gear.stl:
	curl -L -o triple_gear.stl http://www.thingiverse.com/download:182636

test: stltest triple_gear.stl
	./stltest triple_gear.stl

clean:
	rm -f stltest libstl.a *.o triple_gear.stl
