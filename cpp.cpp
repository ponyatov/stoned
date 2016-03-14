#include "hpp.hpp"
#define YYERR "\n\n"<<yylineno<<":"<<msg<<"["<<yytext<<"]\n\n"
void yyerror(string msg) { cout<<YYERR; cerr<<YYERR; exit(-1); }
int main() { env_init(); return yyparse(); }

// abstract symbolic type

Sym::Sym(string T,string V) { tag=T; val=V; }
Sym::Sym(string V):Sym("",V) {}
void Sym::push(Sym*o) { nest.push_back(o); }
void Sym::par(Sym*o) { pars[o->val]=o; }

string Sym::tagval() { return "<"+tag+":"+val+">"; }
string Sym::tagstr() { return "<"+tag+":'"+val+"'>"; }
string Sym::pad(int n) { string S; for (int i=0;i<n;i++) S+='\t'; return S; }
string Sym::dump(int depth) { string S = "\n"+pad(depth)+tagval();
	for (auto pr=pars.begin(),e=pars.end();pr!=e;pr++)
		S += ","+pr->first;
	for (auto it=nest.begin(),e=nest.end();it!=e;it++)
		S += (*it)->dump(depth+1);
	return S; }

Sym* Sym::eval() {
	Sym* E = env[val]; if (E) return E;
	for (auto it=nest.begin(),e=nest.end();it!=e;it++)
		(*it) = (*it)->eval();
	return this; }

Sym* Sym::eq(Sym*o) { env[val]=o; return o; }
Sym* Sym::at(Sym*o) { push(o); return this; }

Sym* Sym::add(Sym*o) { Sym*R = new Op("+");
	R->push(this); R->push(o); return R; }

// scalars

Str::Str(string V):Sym("str",V) {}
Sym* Str::eval() { return this; }
string Str::tagval() { return tagstr(); }

Int::Int(string V):Sym("int",V) { val=atol(V.c_str()); }
Sym* Int::eval() { return this; }
string Int::tagval() { ostringstream os; os<<"<"<<tag<<":"<<val<<">";
	return os.str(); }

Num::Num(string V):Sym("num",V) { val=atof(V.c_str()); }
Sym* Num::eval() { return this; }
string Num::tagval() { ostringstream os; os<<"<"<<tag<<":"<<val<<">";
	return os.str(); }

Hex::Hex(string V):Sym("hex",V) {}
Sym* Hex::eval() { return this; }
Bin::Bin(string V):Sym("bin",V) {}
Sym* Bin::eval() { return this; }

// composites

List::List():Sym("[","]") {}
Vector::Vector():Sym("<",">") {}
Cons::Cons(Sym*A,Sym*B):Sym("","") { push(A); push(B); }

// functionals

Op::Op(string V):Sym("op",V) {}
Sym* Op::eval() {
	if (val=="~") return nest[0]; else Sym::eval();	// quote or nest[]ed eval
	if (val=="=") return nest[0]->eq(nest[1]);		// A = B assign
	if (val=="@") return nest[0]->at(nest[1]);		// A @ B apply
	if (val=="+") return nest[0]->add(nest[1]);		// A + B add
	return this; }

Fn::Fn(string V, FN F):Sym("fn",V) { fn=F; }
Sym* Fn::at(Sym*o) { return fn(o); }

Lambda::Lambda():Sym("^","^") {}
Sym* Lambda::eval() { return this; }

// ext:fileio

Dir::Dir(Sym*o):Sym("dir",o->val) {}
Sym* Dir::dir(Sym*o) { return new Dir(o); }
Sym* Dir::add(Sym*o) {
	Sym*F = o;
	if ((o->tag!="dir")&&(o->tag=="file")) F = new File(o);
	push(F); return F; }

File::File(Sym*o):Sym("file",o->val) {}
Sym* File::file(Sym*o) { return new File(o); }

// global environment

map<string,Sym*> env;
void env_init() {
	env["MODULE"] = new Sym(MODULE);
	// specials
	env["T"] = new Sym("bool","true");
	env["F"] = new Sym("bool","false");
	env["N"] = new Sym("nil","N");
	env["D"] = new Sym("default","D");
	env["E"] = new Sym("error","E");
	// ext:fileio
	env["dir"] = new Fn("dir",Dir::dir);
	env["file"] = new Fn("file",File::file);
}
