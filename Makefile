CXX=g++

ifeq ($(DEBUG), 1)
	CXXFLAGS=-std=c++14 -I. -g -pthread
else
	CXXFLAGS=-std=c++14 -I. -O3 -pthread
endif

LDFLAGS=

PROGRAMS=find_RTP mps_to_smt2 mps_to_smt2_inf normalize_num vipr_to_smt2

all: $(PROGRAMS)

find_RTP: find_RTP.cpp
	$(CXX) $(CXXFLAGS) $(FLAGS) -o $@ $< $(LDFLAGS)
	
mps_to_smt2: mps_to_smt2.cpp
	$(CXX) $(CXXFLAGS) $(FLAGS) -o $@ $< $(LDFLAGS)
	
normalize_num: normalize_num.cpp
	$(CXX) $(CXXFLAGS) $(FLAGS) -o $@ $< $(LDFLAGS)
	
vipr_to_smt2: vipr_to_smt2.cpp
	$(CXX) $(CXXFLAGS) $(FLAGS) -o $@ $< $(LDFLAGS)

mps_to_smt2_inf: mps_to_smt2_inf.cpp
	$(CXX) $(CXXFLAGS) $(FLAGS) -o $@ $< $(LDFLAGS)
	
#%.o: %.c
#	$(CXX) -c -o $@ -I. $<

clean:
	rm -f *.o $(PROGRAMS)
