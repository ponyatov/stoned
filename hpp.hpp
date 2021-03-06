#ifndef _H_STONED
#define _H_STONED

#define AUTHOR "(c) Dmitry Ponyatov <dponyatov@gmail.com>, GNU LGPL, 2011"
#define GITHUB "https://github.com/ponyatov/stoned"

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cassert>
#include <vector>
#include <map>
using namespace std;

// ============================================== Abstract Symbolic Type (AST)
struct Sym {
	string tag,val,doc;									// tag:value:doc
	// --------------------------------------------------- constructors
	Sym(string,string);									// T:V
	Sym(string);										// token
	virtual Sym* cp();									// copy constructor T:V
	// --------------------------------------------------- nest[]ed
	vector<Sym*> nest; void push(Sym*);
	// --------------------------------------------------- par{}ameters
	map<string,Sym*> pars; void par(Sym*);
	// --------------------------------------------------- dumping
	virtual string tagval();							// <T:V>
	string tagstr();									// <T:'V'>
	string pad(int);									// pad tree element
	virtual string dump(int=0);							// dump in tree form
	// --------------------------------------------------- evaluate/compute
	virtual Sym* eval();
	// --------------------------------------------------- operators
	virtual Sym* str();									// str(A)	string repr
	virtual long size();								// size(A)
	virtual Sym* eq(Sym*);								// A = B	assign
	virtual Sym* at(Sym*);								// A @ B	apply
	virtual Sym* inher(Sym*);							// A : B	inherit
	virtual Sym* pfxadd();								// +A
	virtual Sym* pfxsub();								// -A
	virtual Sym* add(Sym*);								// A + B	add
	virtual Sym* dot(Sym*);								// A . B	index
	virtual Sym* ins(Sym*);								// A += B	insert
	//virtual Sym* del(Sym*);								// A -= B	delete
	virtual Sym* smap(Sym*);							// A | B	map
	// --------------------------------------------------- req for lambda apply
	Sym* copy();										// recursive copy
	Sym* replace(string,Sym*);							// rec replace by value
	// --------------------------------------------------- codegen
	virtual Sym* h();
	static Sym* hh(Sym*);
	virtual Sym* c();
	static Sym* cc(Sym*);
};

// ====================================================== GLOBAL ENV{}IRONMENT

extern map<string,Sym*> env;
extern void env_init();

// ================================================================== SPECIALS

extern Sym* T;	// bool:true
extern Sym* F;	// bool:false
extern Sym* N;	// nil:
extern Sym* D;	// default:
extern Sym* E;	// error:
extern Sym* R;	// signal:read
extern Sym* W;	// signal:write

// ================================================================= DIRECTIVE

struct Directive: Sym { Directive(string); string tagval(); };

// =================================================================== SCALARS

struct Scalar: Sym { Scalar(string,string); Sym*eval(); };

struct Str: Scalar { Str(string); Sym*cp();						// string
	string tagval(); Sym*add(Sym*); };
struct Int: Scalar { Int(string); Sym*cp();						// integer
	Int(long); long val; string tagval(); };
struct Num: Scalar { Num(string); Sym*cp();						// floating
	Num(double); double val; string tagval(); };
struct Hex: Scalar { Hex(string); Sym*cp(); };					// hex
struct Bin: Scalar { Bin(string); Sym*cp(); };					// bin str

// ================================================================ COMPOSITES

struct List: Sym { List(); long size(); };						// [list]
struct Vector: Sym { Vector(); };								// <vector>
struct Cons: Sym { Cons(Sym*,Sym*); long size(); };				// co,ns pair

// =============================================================== FUNCTIONALS

// ======================================================= operator
struct Op: Sym { Op(string); Sym*cp(); Sym*eval(); };

// ======================================================= internal function
typedef Sym*(*FN)(Sym*);
struct Fn: Sym { Fn(string,FN); FN fn; Sym*at(Sym*); };

// ======================================================= {la:mbda}
struct Lambda: Sym { Lambda(); Sym*eval(); Sym*at(Sym*); };

// =================================================================== OBJECTS

struct Class: Sym { Class(string); static Sym* clazz(Sym*);
	Sym*at(Sym*); Sym*h(); };

// ================================================================ ext:FILEIO

// ======================================================= directory
struct Dir: Sym { Dir(Sym*); static Sym* dir(Sym*);
	Sym*add(Sym*); Sym*dot(Sym*); };
// ======================================================= file
struct File: Sym { File(Sym*); static Sym* file(Sym*);
	Sym*h(); };

// ==================================================== LEXER/PARSER INTERFACE
// ======================================================= lexer /flex/
extern int yylex();
extern int yylineno;
extern char* yytext;
#define TOC(C,X) { yylval.o = new C(yytext); return X; }
// ======================================================= parser /bison/
extern int yyparse();
extern void yyerror(string);
#include "ypp.tab.hpp"

#endif // _H_STONED
