#include "hpp.hpp"
// ======================================================= error callback
#define YYERR "\n\n"<<yylineno<<":"<<msg<<"["<<yytext<<"]\n\n"
void yyerror(string msg) { cout<<YYERR; cerr<<YYERR; exit(-1); }
// ======================================================= main()
int main() { env_init(); return yyparse(); }

// ============================================== Abstract Symbolic Type (AST)

// ------------------------------------------------------- constructors
Sym::Sym(string T,string V) { tag=T; val=V; }			// <T:V>
Sym::Sym(string V):Sym("",V) {}							// token

// ------------------------------------------------------- nest[]ed elements
void Sym::push(Sym*o) { nest.push_back(o); }

// ------------------------------------------------------- par{}ameters
void Sym::par(Sym*o) { pars[o->val]=o; }

// ------------------------------------------------------- dumping
string Sym::tagval() { return "<"+tag+":"+val+">"; }	// <T:V> header string
string Sym::tagstr() { return "<"+tag+":'"+val+"'>"; }	// <T:'V'> header
string Sym::dump(int depth) {							// dump as text
	string S = "\n"+pad(depth)+tagval();				// <T:V>
	for (auto pr=pars.begin(),e=pars.end();pr!=e;pr++)	// par{}ameters
		S += "\n" + pad(depth+1) \
			+ pr->first \
			+ pr->second->dump(depth+2);
	for (auto it=nest.begin(),e=nest.end();it!=e;it++)	// nest[]ed
		S += (*it)->dump(depth+1);
	return S; }
string Sym::pad(int n) { string S; for (int i=0;i<n;i++) S+='\t'; return S; }

// ------------------------------------------------------- evaluation

Sym* Sym::eval() {
	Sym* E = env[val]; if (E) return E;					// lookup in env{}
	for (auto it=nest.begin(),e=nest.end();it!=e;it++)	// recursive eval()
		(*it) = (*it)->eval();							// with objects replace
	return this; }

// ------------------------------------------------------- operators

Sym* Sym::eq(Sym*o) { env[val]=o; return o; }			// A = B assign

Sym* Sym::at(Sym*o) { push(o); return this; }			// A @ B apply

Sym* Sym::add(Sym*o) { Sym*R = new Op("+");				// A + B add
	R->push(this); R->push(o); return R; }

Sym* Sym::pfxadd() { val = "+"+val; return this; }		// +A
Sym* Sym::pfxsub() { val = "-"+val; return this; }		// -A

// ================================================================= DIRECTIVE
Directive::Directive(string V):Sym("",V) {
	while (val.size() && (val[0]!=' ' && val[0]!='\t')) {
		tag += val[0]; val.erase(0,1); }
	while (val.size() && (val[0]==' ' || val[0]=='\t')) {
		               val.erase(0,1); }				}
string Directive::tagval() { return tagstr(); }

// =================================================================== SCALARS

Scalar::Scalar(string T,string V):Sym(T,V) {};
Sym* Scalar::eval() { return this; }					// block env{} lookup

// ======================================================= string
Str::Str(string V):Scalar("str",V) {}
string Str::tagval() { return tagstr(); }

Int::Int(string V):Scalar("int",V) { val=atol(V.c_str()); }
string Int::tagval() { ostringstream os; os<<"<"<<tag<<":"<<val<<">";
	return os.str(); }

Num::Num(string V):Scalar("num",V) { val=atof(V.c_str()); }
string Num::tagval() { ostringstream os; os<<"<"<<tag<<":"<<val<<">";
	return os.str(); }

Hex::Hex(string V):Scalar("hex",V) {}
Bin::Bin(string V):Scalar("bin",V) {}

// ================================================================ COMPOSITES

List::List():Sym("[","]") {}
Vector::Vector():Sym("<",">") {}
Cons::Cons(Sym*A,Sym*B):Sym("","") { push(A); push(B); }

// =============================================================== FUNCTIONALS

// ======================================================= operator
Op::Op(string V):Sym("op",V) {}
Sym* Op::eval() {
	if (val=="~") return nest[0];					// quote or nest[]ed eval
	else Sym::eval();
	if (val=="=") return nest[0]->eq(nest[1]);		// A = B assign
	if (val=="@") return nest[0]->at(nest[1]);		// A @ B apply
	if (val=="+") return nest[0]->add(nest[1]);		// A + B add
	return this; }

// ======================================================= internal function
Fn::Fn(string V, FN F):Sym("fn",V) { fn=F; }
Sym* Fn::at(Sym*o) { return fn(o); }

// ======================================================= {la:mbda} magic
Lambda::Lambda():Sym("^","^") {}
Sym* Lambda::eval() { return this; }

Sym* Sym::copy() {										// == recursive copy ==
	Sym* R = new Sym(tag,val);							// new empty <T:V>
	for (auto pr=pars.begin(),e=pars.end();pr!=e;pr++)	// copy par{}s
		R->pars[pr->first] = pr->second;				// don't copy par value
	for (auto it=nest.begin(),e=nest.end();it!=e;it++)	// copy nest[]ed
		R->push((*it)->copy());							// recursive _copy_
	return R; }

Sym* Sym::replace(string S,Sym*o) {						// == replace by val ==
	if (val==S) return o;								// object itself -> o
	for (auto pr=pars.begin(),e=pars.end();pr!=e;pr++)	// replace par{}s
		if (pr->first==S) pr->second=o;
	for (auto it=nest.begin(),e=nest.end();it!=e;it++)	// replaces nest[]ed
		(*it)=(*it)->replace(S,o);						// recursive replace
	return this; }

Sym* Lambda::at(Sym*o) {								// == apply by replace
	Sym* R = copy();									// create lambda copy
	for (auto pr=pars.begin(),e=pars.end();pr!=e;pr++)	// loop over params
		R=R->replace(pr->first,o);						// replacing by o
	return R->nest[0]->eval(); }						// ret _evaluated_ copy

// =================================================================== OBJECTS

// ================================================================ ext:FILEIO

// ======================================================= directory
Dir::Dir(Sym*o):Sym("dir",o->val) {}
Sym* Dir::dir(Sym*o) { return new Dir(o); }
Sym* Dir::add(Sym*o) {
	Sym*F = o;
	if ((o->tag!="dir")&&(o->tag=="file")) F = new File(o);
	push(F); return F; }

// ======================================================= file
File::File(Sym*o):Sym("file",o->val) {}
Sym* File::file(Sym*o) { return new File(o); }

// ====================================================== GLOBAL ENV{}IRONMENT

map<string,Sym*> env;
void env_init() {
	// ----------------------------------------------- metainfo constants
	env["MODULE"]	= new Str(MODULE);				// module name
	env["OS"]		= new Str(OS);					// target os
	env["AUTHOR"]	= new Str(AUTHOR);				// author (c)
	env["GITHUB"]	= new Str(GITHUB);				// github://
	// ----------------------------------------------- specials
	env["T"]		= new Sym("bool","true");		// bool:true
	env["F"]		= new Sym("bool","false");		// bool:false
	env["N"]		= new Sym("nil","N");			// nil:
	env["D"]		= new Sym("default","D");		// default:
	env["E"]		= new Sym("error","E");			// error:
	// ----------------------------------------------- fileio
	env["dir"]		= new Fn("dir",Dir::dir);
	env["file"]		= new Fn("file",File::file);
}
