MODULE = $(notdir $(CURDIR))
log.log: ./exe.exe src.src
	./exe.exe < src.src > log.log && tail $(TAIL) log.log
C = cpp.cpp $(OS).cpp ypp.tab.cpp lex.yy.c
H = hpp.hpp $(OS).hpp ypp.tab.hpp
CXXFLAGS += -std=gnu++11 -DMODULE=\"$(MODULE)\" -DOS=\"$(OS)\"
./exe.exe: $(C) $(H)
	$(CXX) $(CXXFLAGS) -o $@ $(C)
ypp.tab.cpp: ypp.ypp
	bison $<
lex.yy.c: lpp.lpp
	flex $<