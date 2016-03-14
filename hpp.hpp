#ifndef _H_STONED
#define _H_STONED

#include <iostream>
#include <cstdlib>
#include <vector>
#include <map>
using namespace std;

struct Sym {
	string tag,val;
	Sym(string,string); Sym(string);
	vector<Sym*> nest; void push(Sym*);
//	map<string,Sym*> pars; void par(Sym*);
	virtual string tagval();
	virtual string dump(int=0); string pad(int);
	virtual Sym* eval();
//	virtual Sym* eq(Sym*);
//	virtual Sym* at(Sym*);
};

extern map<string,Sym*> env;
extern void env_init();

extern int yylex();
extern int yylineno;
extern char* yytext;
#define TOC(C,X) { yylval.o = new C(yytext); return X; }
extern int yyparse();
extern void yyerror(string);
#include "ypp.tab.hpp"

#endif // _H_STONED
