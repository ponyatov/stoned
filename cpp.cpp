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
Sym* Sym::cp() { return new Sym(tag,val); }				// copy T:V

// ------------------------------------------------------- nest[]ed elements
void Sym::push(Sym*o) { nest.push_back(o); }

// ------------------------------------------------------- par{}ameters
void Sym::par(Sym*o) { pars[o->str()->val]=o; }

// ------------------------------------------------------- dumping
string Sym::tagval() { return "<"+tag+":"+val+"> "+doc; }// <T:V> header
string Sym::tagstr() { string S = "<"+tag+":'";			// <T:'V'> header
	for (int i=0,n=val.length();i<n;i++) {
		char c=val[i]; switch (c) {
		case '\t': S+="\\t"; break;
		case '\n': S+="\\n"; break;
		default: S+=c;
		}}
	return S+"'> "+doc; }
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

// -------------------------------------------------

Sym* Sym::str()		{ return new Str(val); }			// str(A) string repr
Sym* Sym::eq(Sym*o) { env[str()->val]=o; return o; }	// A = B assign
Sym* Sym::at(Sym*o) { push(o); return this; }			// A @ B apply

Sym* Sym::inher(Sym*o) {								// A : B inherit
	return new Sym(str()->val,o->str()->val); }

Sym* Sym::add(Sym*o) { return new Sym(val+o->str()->val); }	// A + B add

Sym* Sym::pfxadd() { val = "+"+val; return this; }		// +A
Sym* Sym::pfxsub() { val = "-"+val; return this; }		// -A

Sym* Sym::ins(Sym*o) { push(o); return this; }			// A+=B insert

long Sym::size()	{ return 1; }						// size(A)

Sym* Sym::smap(Sym*o) {
	List* R = new List();
	for (auto it=o->nest.begin(),e=o->nest.end();it!=e;it++)
		R->push(at(*it));
	return R;
}

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
Sym* Str::cp() { return new Str(val); }
string Str::tagval() { return tagstr(); }
Sym* Str::add(Sym*o) { return new Str(val+o->str()->val); }

// ======================================================= integer
Int::Int(string V):Scalar("int",V) { val=atol(V.c_str()); }
Int::Int(long L):Scalar("int","") { val=L; }
Sym* Int::cp() { return new Int(val); }
string Int::tagval() { ostringstream os; os<<"<"<<tag<<":"<<val<<">";
	return os.str(); }

// ======================================================= floating number
Num::Num(string V):Scalar("num",V) { val=atof(V.c_str()); }
Num::Num(double D):Scalar("num","") { val=D; }
Sym* Num::cp() { return new Num(val); }
string Num::tagval() { ostringstream os; os<<"<"<<tag<<":"<<val<<">";
	return os.str(); }

// ======================================================= machine hex
Hex::Hex(string V):Scalar("hex",V) {}
Sym* Hex::cp() { return new Hex(val); }
// ======================================================= binary string
Bin::Bin(string V):Scalar("bin",V) {}
Sym* Bin::cp() { return new Bin(val); }

// ================================================================ COMPOSITES

// ======================================================= [list]
List::List():Sym("[","]") {}
long List::size() { return nest.size(); }

// ======================================================= <vector>
Vector::Vector():Sym("<",">") {}

// ======================================================= co,ns
Cons::Cons(Sym*A,Sym*B):Sym("","") { push(A); push(B); }
long Cons::size() { return 2; }//return 1+nest[1]->size(); }

// =============================================================== FUNCTIONALS

// ======================================================= operator
Op::Op(string V):Sym("op",V) {}
Sym* Op::cp() { return new Op(val); }
Sym* Op::eval() {
	if (val=="~") return nest[0];					// quote or nest[]ed eval
	else Sym::eval();
	if (val=="=") return nest[0]->eq(nest[1]);		// A = B assign
	if (val=="@") return nest[0]->at(nest[1]);		// A @ B apply
	if (val==":") return nest[0]->inher(nest[1]);	// A : B inherit
	if (val=="+") return nest[0]->add(nest[1]);		// A + B add
	if (val=="+=") return nest[0]->ins(nest[1]);	// A += B insert
	if (val=="|") return nest[0]->smap(nest[1]);	// A|B map
	return this; }

// ======================================================= internal function
Fn::Fn(string V, FN F):Sym("fn",V) { fn=F; }
Sym* Fn::at(Sym*o) { return fn(o); }

// ======================================================= {la:mbda} magic
Lambda::Lambda():Sym("^","^") {}
Sym* Lambda::eval() { return this; }

Sym* Sym::copy() {										// == recursive copy ==
	Sym* R = cp();										// copy object T:V
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
	//cerr << pars.size() << " " << o->size() << "\n";
	assert(pars.size()==o->size());
	int i=0; for (auto pr=pars.begin(),e=pars.end();pr!=e;pr++,i++)
		if (o->size()==1)
			R=R->replace(pr->first,o);					// replacing by o
		else {
			R=R->replace(pr->first,o->nest[i]);			// replacing by o
		}
	return R->nest[0]->eval(); }						// ret _evaluated_ copy

// =================================================================== OBJECTS

Class::Class(string V):Sym("class",V) {}
Sym* Class::clazz(Sym*o) { return new Class(o->str()->val); }
Sym* Class::at(Sym*o) { return new Sym(val,o->str()->val); }

// ================================================================ ext:FILEIO

// ======================================================= directory
Dir::Dir(Sym*o):Sym("dir",o->str()->val) {}
Sym* Dir::dir(Sym*o) { return new Dir(o); }
Sym* Dir::add(Sym*o) {
	Sym*F = o;
	if ((o->tag!="dir")&&(o->tag!="file")) F = new File(o);
	push(F); return F; }

// ====================================================== file
File::File(Sym*o):Sym("file",o->str()->val) {}
Sym* File::file(Sym*o) { return new File(o); }
Sym* File::h() { return new Str("#include \""+val+"\""); }

// ==================================================================== CODEGEN

Sym* Sym::hh(Sym*o) { return o->h(); }
Sym* Sym::h()		{ return new Str("extern "+tag+' '+val+';'); }
Sym* Sym::cc(Sym*o) { return o->c(); }
Sym* Sym::c()		{ return new Str(tag+' '+val+';'); }

// ======================================================= GLOBAL ENV{}IRONMENT

map<string,Sym*> env;
void env_init() {
	// ----------------------------------------------- metainfo constants
	env["MODULE"]	= new Str(MODULE);				// module name
	env["OS"]		= new Str(OS);					// target os
	env["AUTHOR"]	= new Str(AUTHOR);				// author (c)
	env["GITHUB"]	= new Str(GITHUB);				// github://
	// ----------------------------------------------- specials
	env["T"]		= new Sym("bool","T");			// bool:true
	env["F"]		= new Sym("bool","F");			// bool:false
	env["N"]		= new Sym("nil","N");			// nil:
	env["D"]		= new Sym("default","D");		// default:
	env["E"]		= new Sym("error","E");			// error:
	// ----------------------------------------------- fileio
	env["dir"]		= new Fn("dir",Dir::dir);		// directory
	env["file"]		= new Fn("file",File::file);	// file
	// ----------------------------------------------- class subsystem
	env["class"]	= new Fn("class",Class::clazz);
	// ----------------------------------------------- codegen
	env["h"]		= new Fn("h",Sym::hh);
	env["c"]		= new Fn("c",Sym::cc);
}
