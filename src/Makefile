# Copyright Paul Dardeau, SwampBits LLC 2022

CC = c++
CC_OPTS = -c -Wall -fPIC -O2 -I../../chaudiere/src -I../../chapeau/src
LINK_CMD = c++
# remove -ldl for non-linux
LINK_LIBS = -lsqlite3

EXE_NAME = cpp-cloud-jukebox

OBJS =  argument_parser.o \
fs_storage_system.o \
jb_utils.o \
jukebox.o \
jukebox_db.o \
jukebox_main.o \
main.o \
property_set.o \
utils.o


all : $(EXE_NAME)

clean :
	rm -f *.o
	rm -f $(EXE_NAME)

$(EXE_NAME) : $(OBJS)
	$(LINK_CMD) $(OBJS) -o $(EXE_NAME) ../../chaudiere/src/libchaudiere.so ../../chapeau/src/libchapeau.so $(LINK_LIBS)

%.o : %.cpp
	$(CC) $(CC_OPTS) $< -o $@
