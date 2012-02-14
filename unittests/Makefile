CPP=g++
CPPFLAGS=-O3 -Wall -g -I../
LDFLAGS=-lcrypto -L../

TARGET=testcondition testparse testsblt

all: $(TARGET)

testcondition: testcondition.cpp ../conditions.o ../util.o ../regexp.o ../sblookuptable.o
	$(CPP) $(CPPFLAGS) $(LDFLAGS) -o $@ $^
 
testsblt: testsblt.cpp ../conditions.o ../util.o ../regexp.o ../sblookuptable.o
	$(CPP) $(CPPFLAGS) $(LDFLAGS) -o $@ $^

testparse: testparse.cpp ../conditions.o ../util.o ../regexp.o ../sblookuptable.o
	$(CPP) $(CPPFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm -f *.o $(TARGET)
