# Copyright Paul Dardeau, SwampBits LLC 2022

CC = c++
CC_OPTS = -c -Wall -std=c++20 -fPIC -g -pthread -I../include -I../chapeau/chaudiere/src -I../chapeau/src
LINK_CMD = c++
# remove -ldl for non-linux
# libapps.a    libcrypto.a  libcurlpp.a   libinih.a    libminiocpp.a  libssl.a
# libcommon.a  libcurl.a    libdefault.a  liblegacy.a  libpugixml.a   libz.a
LINK_LIBS = -lsqlite3 -lz

EXE_NAME = cpp-cloud-jukebox

OBJS =  argument_parser.o \
fs_storage_system.o \
jb_utils.o \
jukebox.o \
jukebox_db.o \
jukebox_main.o \
main.o \
mirror_storage_system.o \
property_set.o \
song_downloader.o \
s3ext_storage_system.o \
utils.o


all : $(EXE_NAME)

clean :
	rm -f *.o
	rm -f $(EXE_NAME)

$(EXE_NAME) : $(OBJS)
	$(LINK_CMD) $(OBJS) -o $(EXE_NAME) -L../lib -lchaudiere -L../lib -lchapeau -L/usr/local/lib -L../lib $(LINK_LIBS)

%.o : %.cpp
	$(CC) $(CC_OPTS) $< -o $@
