CC = c++
CC_OPTS = -c -I../src -I../../chaudiere/src -I../../chapeau/src -I../../poivre

EXE_NAME = test_cpp_cloud_jukebox
LIB_NAMES = ../../poivre/poivre.so ../../chaudiere/src/libchaudiere.so ../../chapeau/src/libchapeau.so -L../lib -lsqlite3 -lssl -lcrypto

PROJ_OBJS = ../src/utils.o \
../src/property_set.o \
../src/argument_parser.o \
../src/jukebox_db.o \
../src/jb_utils.o \
../src/fs_storage_system.o \
../src/jukebox.o

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
