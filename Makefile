MODULE = $(notdir $(CURDIR))
# OS = win32|unix
OS ?= unix
# TAIL = -n17|-n7|<none>

################ minimal lex program Makefile ################
log.log: ./exe.exe src.src lude.test
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
###############################################################
	
.PHONY: clean
clean:
	rm -rf ./exe.exe log.log ypp.tab.?pp lex.yy.c
	
lude.test:
	touch $@
