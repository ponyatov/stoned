#ifndef _H_STONED
#define _H_STONED

#define AUTHOR "(c) Dmitry Ponyatov <dponyatov@gmail.com>, GNU LGPL, 2011"
#define GITHUB "https://github.com/ponyatov/stoned"

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <map>
using namespace std;

// ============================================== Abstract Symbolic Type (AST)
struct Sym {
	string tag,val;										// tag:value
	// --------------------------------------------------- constructors
	Sym(string,string);									// T:V
	Sym(string);										// token
	// --------------------------------------------------- nest[]ed
	vector<Sym*> nest; void push(Sym*);
	// --------------------------------------------------- par{}ameters
	map<string,Sym*> pars; void par(Sym*);
	// --------------------------------------------------- dumping
	virtual string tagval(); string tagstr();
	virtual string dump(int=0); string pad(int);
	// --------------------------------------------------- evaluate/compute
	virtual Sym* eval();
	// --------------------------------------------------- operators
	virtual Sym* eq(Sym*);								// A = B assign
	virtual Sym* at(Sym*);								// A @ B apply
	virtual Sym* add(Sym*);								// A + B add
};

// ====================================================== GLOBAL ENV{}IRONMENT

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

// ================================================================ COMPOSITES

struct List: Sym { List(); };									// [list]
struct Vector: Sym { Vector(); };								// <vector>
struct Cons: Sym { Cons(Sym*,Sym*); };							// co,ns pair

// =============================================================== FUNCTIONALS

// ======================================================= operator
struct Op: Sym { Op(string); Sym*eval(); };

// ======================================================= internal function
typedef Sym*(*FN)(Sym*);
struct Fn: Sym { Fn(string,FN); FN fn; Sym*at(Sym*); };

// ======================================================= {la:mbda}
struct Lambda: Sym { Lambda(); Sym*eval(); };

// =================================================================== OBJECTS

// ================================================================ ext:FILEIO

// ======================================================= directory
struct Dir: Sym { Dir(Sym*); static Sym* dir(Sym*);
	Sym*add(Sym*); };
// ======================================================= file
struct File: Sym { File(Sym*); static Sym* file(Sym*); };

extern int yylex();
extern int yylineno;
extern char* yytext;
#define TOC(C,X) { yylval.o = new C(yytext); return X; }
extern int yyparse();
extern void yyerror(string);
#include "ypp.tab.hpp"

#endif // _H_STONED
