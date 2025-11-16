CC = c++
CC_OPTS = -c -I../src -I../chapeau/chaudiere/src -I../chapeau/src

EXE_NAME = test_cpp_cloud_jukebox
LIB_NAMES = -L../lib -lchaudiere -L../lib -lchapeau -L../lib -lsqlite3

PROJ_OBJS = ../src/utils.o \
../src/property_set.o \
../src/argument_parser.o \
../src/jukebox_db.o \
../src/jb_utils.o \
../src/fs_storage_system.o \
../src/jukebox.o \
../src/song_downloader.o

OBJS = test_utils.o \
fs_test_case.o \
test_property_set.o \
test_argument_parser.o \
test_jukebox_db.o \
test_s3_storage_system.o \
test_fs_storage_system.o \
test_jukebox.o \
tests.o

all : $(EXE_NAME)

clean :
	rm -f *.o
	rm -f $(EXE_NAME)
	
$(EXE_NAME) : $(OBJS)
	$(CC) $(OBJS) $(PROJ_OBJS) -o $(EXE_NAME) $(LIB_NAMES)

%.o : %.cpp
	$(CC) $(CC_OPTS) $< -o $@
