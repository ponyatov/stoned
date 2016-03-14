#ifndef _H_STONED
#define _H_STONED

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <map>
using namespace std;

struct Sym {
	string tag,val;
	Sym(string,string); Sym(string);
	vector<Sym*> nest; void push(Sym*);
//	map<string,Sym*> pars; void par(Sym*);
	virtual string tagval(); string tagstr();
	virtual string dump(int=0); string pad(int);
	virtual Sym* eval();
	virtual Sym* eq(Sym*);
//	virtual Sym* at(Sym*);
};

extern map<string,Sym*> env;
extern void env_init();

struct Str: Sym { Str(string); Sym*eval(); string tagval(); };
struct Int: Sym { Int(string); Sym*eval(); long val; string tagval(); };
struct Num: Sym { Num(string); Sym*eval(); double val; string tagval(); };
struct Hex: Sym { Hex(string); Sym*eval(); };
struct Bin: Sym { Bin(string); Sym*eval(); };

struct List: Sym { List(); };
struct Vector: Sym { Vector(); };
struct Cons: Sym { Cons(Sym*,Sym*); };

struct Op: Sym { Op(string); Sym*eval(); };

struct Lambda: Sym { Lambda(); };

extern int yylex();
extern int yylineno;
extern char* yytext;
#define TOC(C,X) { yylval.o = new C(yytext); return X; }
extern int yyparse();
extern void yyerror(string);
#include "ypp.tab.hpp"

#endif // _H_STONED
