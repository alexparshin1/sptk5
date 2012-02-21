include ../config.mak

LIBNAME = libtar.a
OBJECTS = block.o decode.o handle.o libtar_hash.o libtar_list.o util.o
CFLAGS += -I. -I.. -I../common -I../include
CXXFLAGS += -I. -I.. -I../common -I../include

$(LIBNAME): $(OBJECTS)
	$(RM) $(LIBNAME)
	$(AR) $(LIBNAME) $(OBJECTS)

clean:
	$(RM) $(LIBNAME) *.o
