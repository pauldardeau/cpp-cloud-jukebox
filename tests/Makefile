CC = c++
CC_OPTS = -c -I../src -I../../chaudiere/src -I../../poivre

EXE_NAME = test_cpp_cloud_jukebox
LIB_NAMES = ../../poivre/poivre.so ../../chaudiere/src/libchaudiere.so -L../lib -lssl -lcrypto

PROJ_OBJS = ../src/utils.o

OBJS = test_utils.o \
tests.o

all : $(EXE_NAME)

clean :
	rm -f *.o
	rm -f $(EXE_NAME)
	
$(EXE_NAME) : $(OBJS)
	$(CC) $(OBJS) $(PROJ_OBJS) -o $(EXE_NAME) $(LIB_NAMES)

%.o : %.cpp
	$(CC) $(CC_OPTS) $< -o $@
