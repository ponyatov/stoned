#include "hpp.hpp"
#define YYERR "\n\n"<<yylineno<<":"<<msg<<"["<<yytext<<"]\n\n"
void yyerror(string msg) { cout<<YYERR; cerr<<YYERR; exit(-1); }
int main() { env_init(); return yyparse(); }

Sym::Sym(string T,string V) { tag=T; val=V; }
Sym::Sym(string V):Sym("",V) {}
void Sym::push(Sym*o) { nest.push_back(o); }

string Sym::tagval() { return "<"+tag+":"+val+">"; }
string Sym::tagstr() { return "<"+tag+":'"+val+"'>"; }
string Sym::pad(int n) { string S; for (int i=0;i<n;i++) S+='\t'; return S; }
string Sym::dump(int depth) { string S = "\n"+pad(depth)+tagval();
//	for (auto pr=pars.begin(),e=pars.end();pr!=e;pr++)
//		S += ","+pr->first;
	for (auto it=nest.begin(),e=nest.end();it!=e;it++)
		S += (*it)->dump(depth+1);
	return S; }

Sym* Sym::eval() {
	Sym* E = env[val]; if (E) return E;
	for (auto it=nest.begin(),e=nest.end();it!=e;it++)
		(*it) = (*it)->eval();
	return this; }

Sym* Sym::eq(Sym*o) { env[val]=o; return o; }

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

List::List():Sym("[","]") {}
Vector::Vector():Sym("<",">") {}
Cons::Cons(Sym*A,Sym*B):Sym("","") { push(A); push(B); }

Op::Op(string V):Sym("op",V) {}
Sym* Op::eval() { Sym::eval();
	if (val=="=") return nest[0]->eq(nest[1]);
	return this; }

Lambda::Lambda():Sym("^","^") {}

map<string,Sym*> env;
void env_init() {
	env["MODULE"] = new Sym(MODULE);
	// specials
	env["T"] = new Sym("bool","true");
	env["F"] = new Sym("bool","false");
	env["N"] = new Sym("nil","N");
	env["D"] = new Sym("default","D");
	env["E"] = new Sym("error","E");
}
