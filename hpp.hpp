#ifndef _H_STONED
#define _H_STONED

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <map>
using namespace std;

// abstract symbolic type

struct Sym {
	string tag,val;
	Sym(string,string); Sym(string);
	vector<Sym*> nest; void push(Sym*);
	map<string,Sym*> pars; void par(Sym*);
	virtual string tagval(); string tagstr();
	virtual string dump(int=0); string pad(int);
	virtual Sym* eval();
	virtual Sym* eq(Sym*);	// A = B assign
	virtual Sym* at(Sym*);	// A @ B apply
	virtual Sym* add(Sym*);	// A + B add
};

// global environment

extern map<string,Sym*> env;
extern void env_init();

// ================================================================= DIRECTIVE

struct Directive: Sym { Directive(string); string tagval(); };

// =================================================================== SCALARS

struct Scalar: Sym { Scalar(string,string); Sym*eval(); };

struct Str: Scalar { Str(string); string tagval(); };				// string
struct Int: Scalar { Int(string); long val; string tagval(); };		// integer
struct Num: Scalar { Num(string); double val; string tagval(); };	// floating
struct Hex: Scalar { Hex(string); };								// hex
struct Bin: Scalar { Bin(string); };								// bin str

// composites

struct List: Sym { List(); };
struct Vector: Sym { Vector(); };
struct Cons: Sym { Cons(Sym*,Sym*); };

// functionals

struct Op: Sym { Op(string); Sym*eval(); };
typedef Sym*(*FN)(Sym*);
struct Fn: Sym { Fn(string,FN); FN fn; Sym*at(Sym*); };
struct Lambda: Sym { Lambda(); Sym*eval(); };

// ext:fileio

struct Dir: Sym { Dir(Sym*); static Sym* dir(Sym*); Sym*add(Sym*); };
struct File: Sym { File(Sym*); static Sym* file(Sym*); };

extern int yylex();
extern int yylineno;
extern char* yytext;
#define TOC(C,X) { yylval.o = new C(yytext); return X; }
extern int yyparse();
extern void yyerror(string);
#include "ypp.tab.hpp"

#endif // _H_STONED
