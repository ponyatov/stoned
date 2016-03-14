MODULE = $(notdir $(CURDIR))
# OS = win32|linux
# TAIL = -n17|-n7|<none>

.PHONY: all
all: log.log
	tail $(TAIL) log.log
	
.PHONY: clean
clean:
	rm -rf ./exe.exe log.log ypp.tab.?pp lex.yy.c
	
################ minimal lex program Makefile ################
log.log: ./exe.exe src.src
	./exe.exe < src.src > log.log 
C = cpp.cpp $(OS).cpp ypp.tab.cpp lex.yy.c
H = hpp.hpp $(OS).hpp ypp.tab.hpp
CXXFLAGS += -std=gnu++11 -DMODULE=\"$(MODULE)\" -DOS=\"$(OS)\"
./exe.exe: $(C) $(H)
	$(CXX) $(CXXFLAGS) -o $@ $(C)
ypp.tab.cpp: ypp.ypp
	bison $<
lex.yy.c: lpp.lpp
	flex $<
###############################################################