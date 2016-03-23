#include "unix.hpp"

Sym* Dir::dot(Sym*o) {
	if (o==W)
		if (0==mkdir(val.c_str(),0700)) return T; else return E;
	else return env["E"];
}
