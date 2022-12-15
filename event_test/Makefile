CC=g++
CXXFLAGS=-Wall -std=c++17 -g -fsanitize=address
INCLUDES=
LDFLAGS=-fsanitize=address
LIBS=-pthread -lrdmacm -libverbs -lboost_iostreams -lboost_program_options

SRCS=main.cpp config.cpp setup.cpp sock.cpp controller.cpp invoker.cpp ib.cpp
OBJS=$(SRCS:.cpp=.o)
PROG=test

all: $(PROG)

debug: CXXFLAGS=-Wall -std=c++17 -g -DDEBUG -fsanitize=address
debug: LDFLAGS=-fsanitize=address
debug: $(PROG)

data: CXXFLAGS=-Wall -std=c++17 -g -DDATA
data: $(PROG)

data_debug: CXXFLAGS=-Wall -std=c++17 -g -DDATA -DDEBUG -fsanitize=address
data_debug: LDFLAGS=-fsanitize=address
data_debug: $(PROG)

.cpp.o:
	$(CC) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

$(PROG): $(OBJS)
	$(CC) $(CXXFLAGS) $(INCLUDES) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)

clean:
	$(RM) *.o *~ $(PROG)
