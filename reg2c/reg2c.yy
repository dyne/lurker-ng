%{

#include <set>
#include <string>
#include <map>
#include <vector>
#include <stdio.h>
#include <string.h>

typedef unsigned char	Input;
typedef set<Input>	Inputs;

const char*	func;
const Input*	regexp;

/************************************************************ Code for NFAs */

typedef unsigned long	NFA_State;
typedef set<NFA_State>	NFA_States;

struct NFA_Rule
{
	Input	ch;
	NFA_States	epsilon;
	NFA_States	transition;
	
	NFA_Rule(Input c) : ch(c) { }
};

const NFA_State START  = 0; /* 0 is start */
const NFA_State ACCEPT = 1; /* 1 is accept */
typedef vector<NFA_Rule> NFA;

bool scan_backwards = false;

/********************** Helper regexp functions */

void display(const NFA& nfa)
{
	NFA_State i;
	NFA_States::iterator j;
	for (i = 0; i < nfa.size(); i++)
	{
		printf("%ld: %c [", i, nfa[i].ch?nfa[i].ch:'.');
		for (j =  nfa[i].transition.begin();
		     j != nfa[i].transition.end();
		     j++)
		{
			printf(" %ld", *j);
		}
		printf(" ] & [");
		for (j =  nfa[i].epsilon.begin();
		     j != nfa[i].epsilon.end();
		     j++)
		{
			printf(" %ld", *j);
		}
		printf(" ]\n");
	}
	
}

void copy_into(NFA& dest, const NFA& source)
{
	NFA       ::const_iterator i;
	NFA_States::const_iterator j;
	
	NFA_State start = dest.size();
	
	for (i = source.begin(); i != source.end(); i++)
	{
		dest.push_back(NFA_Rule(i->ch));
		
		for (j = i->epsilon.begin(); j != i->epsilon.end(); j++)
			dest.back().epsilon.insert(*j + start);
		for (j = i->transition.begin(); j != i->transition.end(); j++)
			dest.back().transition.insert(*j + start);
	}
}

void expand_epsilon(const NFA& nfa, NFA_States& states)
{
	NFA_States touched;
	NFA_States::const_iterator i;
	
	while (!states.empty())
	{
		NFA_State what = *states.begin();
		
		touched.insert(what);
		states.erase(states.begin());
		
		for (i =  nfa[what].epsilon.begin(); 
		     i != nfa[what].epsilon.end(); 
		     i++)
		{
			if (touched.find(*i) == touched.end())
				states.insert(*i);
		}
	}
	
	states.clear();
	for (i = touched.begin(); i != touched.end(); i++)
		if (!nfa[*i].transition.empty() || *i == ACCEPT)
			states.insert(*i);
}

/********************** Required regexp keywords */

NFA nothing()
{
	NFA out;
	
	out.push_back(NFA_Rule(0));
	out.push_back(NFA_Rule(0));
	
	return out;
}

NFA empty()
{
	NFA out;
	
	out.push_back(NFA_Rule(0));
	out.push_back(NFA_Rule(0));
	
	out[START].epsilon.insert(ACCEPT);
	
	return out;
}

NFA token(Input c)
{
	NFA out;
	
	out.push_back(NFA_Rule(c));
	out.push_back(NFA_Rule(0));
	
	out[START].transition.insert(ACCEPT);
	
	return out;
}

NFA or(const NFA& x, const NFA& y)
{
	NFA out;
	NFA_State sx, sy;
	
	out.push_back(NFA_Rule(0));
	out.push_back(NFA_Rule(0));
	
	sx = out.size();
	copy_into(out, x);
	sy = out.size();
	copy_into(out, y);
	
	out[START]    .epsilon.insert(sx+START);
	out[START]    .epsilon.insert(sy+START);
	out[sx+ACCEPT].epsilon.insert(ACCEPT);
	out[sy+ACCEPT].epsilon.insert(ACCEPT);
	
	return out;
}

NFA concat(const NFA& x, const NFA& y)
{
	NFA out;
	NFA_State sx, sy;
	
	out.push_back(NFA_Rule(0));
	out.push_back(NFA_Rule(0));
	
	sx = out.size();
	copy_into(out, x);
	sy = out.size();
	copy_into(out, y);

	if (!scan_backwards)
	{
		out[START]    .epsilon.insert(sx+START);
		out[sx+ACCEPT].epsilon.insert(sy+START);
		out[sy+ACCEPT].epsilon.insert(ACCEPT);
	}
	else
	{
		out[START]    .epsilon.insert(sy+START);
		out[sy+ACCEPT].epsilon.insert(sx+START);
		out[sx+ACCEPT].epsilon.insert(ACCEPT);
	}
	
	return out;
}

NFA star(const NFA& x)
{
	NFA out;
	NFA_State sx;
	
	out.push_back(NFA_Rule(0));
	out.push_back(NFA_Rule(0));
	
	sx = out.size();
	copy_into(out, x);
	
	out[START]    .epsilon.insert(ACCEPT);
	out[START]    .epsilon.insert(sx+START);
	out[sx+ACCEPT].epsilon.insert(START);
	
	return out;
}

//!!! not (!) - implement
//!!! and (&) - implement: (ab)+.*&bb

/********************** Handy regexp keywords */

NFA range(int a, int b)
{	/* Slightly less states than (a|b|c|d|...) */
	NFA out;
	
	out.push_back(NFA_Rule(0));
	out.push_back(NFA_Rule(0));

	while (a != b)
	{
		out[START].epsilon.insert(out.size());
		out.push_back(NFA_Rule(a++));
		out.back().transition.insert(ACCEPT);
	}
	
	return out;
}

NFA range(const NFA& a, const NFA& b)
{
	return range(a[START].ch, b[START].ch+1);
}

NFA onemore(const NFA& x)
{	// a+ = aa*
	return concat(x, star(x));
}

NFA option(const NFA& x)
{	// a? = (#|a)
	return or(empty(), x);
}

void compile(const NFA& nfa);

/************************************************************ Code for parser */

#define YYSTYPE NFA

int yylex()
{
	switch (*regexp)
	{
	case 0:
		return 0;
	
	case '%':
	case '#':
	case '.':
	
	case '*':
	case '?':
	case '+':
	
	case '(':
	case ')':
	
	case '|':
	
	case '[':
	case ']':
	case '-':
		return *regexp++;
		
	case '\\':
		regexp++;
	default:
		regexp++;
		return TOKEN;
	}
}

void yyerror(const char* error)
{
	fprintf(stderr, "%s\n", error);
}

%}

%token TOKEN

%right '!'
%left '+' '*' '?'

%%

input:	regex			{ compile($1); }
;

regao:	TOKEN			{ $$ = token(*(regexp-1)); }
	| '%'			{ $$ = token('%'); }
	| '#'			{ $$ = token('#'); }
	| '.'			{ $$ = token('.'); }
	| '*'			{ $$ = token('*'); }
	| '?'			{ $$ = token('?'); }
	| '+'			{ $$ = token('+'); }
	| '('			{ $$ = token('('); }
	| ')'			{ $$ = token(')'); }
	| '['			{ $$ = token('['); }
	| '|'			{ $$ = token('|'); }
/*!!! the \ is broken */
;

regel:	regao '-' regao		{ $$ = range($1, $3); }
	| regao			{ $$ = $1; }
;

regar:	regel			{ $$ = $1; }
	| regar regel		{ $$ = or($1, $2); }
;

rege:	TOKEN			{ $$ = token(*(regexp-1)); }
	| '-'			{ $$ = token('-'); }
	| '.'			{ $$ = range(0, 256); }
	| '#'			{ $$ = empty(); }
	| '%'			{ $$ = nothing(); }
	| rege '*'		{ $$ = star($1); }
	| rege '?'		{ $$ = option($1); }
	| rege '+'		{ $$ = onemore($1); }
	| '(' regex ')'		{ $$ = $2; }
	| '[' regar ']'		{ $$ = $2; }
;

regcat:	rege			{ $$ = $1; }
	| regcat rege		{ $$ = concat($1, $2); }
;

regex:	regcat			{ $$ = $1; }
	| regex '|' regcat	{ $$ = or($1, $3); }
;

%%

/************************************************************ Code for DFAs */

typedef basic_string<NFA_State> 	DFA_State;
typedef map<Input, DFA_State>		Transition;
typedef	map<DFA_State, Transition>	DFA;

void display(const DFA_State& state)
{
	DFA_State::const_iterator i;
	for (i = state.begin(); i != state.end(); i++)
		printf("%c", (char)('a' + *i));
}

void display(const Transition& t)
{
	Transition::const_iterator i;
	for (i = t.begin(); i != t.end(); i++)
	{
		printf("\t%c: ", i->first);
		display(i->second);
		printf("\n");
	}
}

void display(const DFA& dfa)
{
	DFA::const_iterator d;
	for (d = dfa.begin(); d != dfa.end(); d++)
	{
		display(d->first);
		printf(":\n");
		display(d->second);
	}
}

DFA_State flatten(const NFA_States& states)
{
	DFA_State out;
	NFA_States::iterator i;
	
	for (i = states.begin(); i != states.end(); i++)
		out.push_back(*i);
	
	return out;
}

NFA_States take_branch(const NFA& nfa, const DFA_State& state, Input c)
{
	NFA_States out;
	DFA_State::const_iterator i;
	
	for (i = state.begin(); i != state.end(); i++)
	{
		const NFA_Rule& n = nfa[*i];
		if (c == n.ch)
			out.insert(n.transition.begin(), n.transition.end());
	}
	
	return out;
}

Inputs examine_exits(const NFA& nfa, const DFA_State& state)
{
	Inputs out;
	DFA_State::const_iterator i;
	
	for (i = state.begin(); i != state.end(); i++)
	{
		const NFA_Rule& n = nfa[*i];
		out.insert(n.ch);
	}
	
	return out;
}

Transition transition(const NFA& nfa, const DFA_State& what)
{
	Transition out;
	Inputs inputs = examine_exits(nfa, what);
	
	Inputs::iterator i;
	for (i = inputs.begin(); i != inputs.end(); i++)
	{
		NFA_States where = take_branch(nfa, what, *i);
		expand_epsilon(nfa, where);
		DFA_State dest = flatten(where);
		
		if (dest.length())
			out[*i] = dest;
	}
	
	return out;
}

DFA dedeterministic_it(const NFA& nfa, DFA_State& start)
{
	DFA		out;
	set<DFA_State>	todo;
	
	NFA_States where;
	where.insert(START);
	expand_epsilon(nfa, where);
	
	todo.insert(start = flatten(where));
	todo.insert(DFA_State());
	while (!todo.empty())
	{
		DFA_State what = *todo.begin();
		todo.erase(todo.begin());

		Transition& t = out[what] = transition(nfa, what);
		Transition::iterator i;
		for (i = t.begin(); i != t.end(); i++)
		{
			if (out.find(i->second) == out.end())
				todo.insert(i->second);
		}
	}
	
	return out;
}

bool is_final(const DFA_State& dfa)
{
	return (dfa.size() > 0 && dfa[0] == ACCEPT) ||
	       (dfa.size() > 1 && dfa[1] == ACCEPT);
}

/************************************************************ Reduce states */

vector<DFA_State>			reduced_states;
typedef vector<DFA_State>::size_type	CState;
map<DFA_State, CState>			reduced_map;

vector<vector<bool> >			marks;

bool differ(const Transition& a, const Transition& b)
{
	Transition::const_iterator ia, ib;
	
	for (ia  = a.begin(), ib  = b.begin();
	     ia != a.end() && ib != b.end();
	     ia++,            ib++)
	{
		if (ia->first != ib->first) return true;
		
		CState ca = reduced_map[ia->second];
		CState cb = reduced_map[ib->second];
		
		if ((ca > cb && marks[ca][cb]) ||
                    (cb > ca && marks[cb][ca]))
			return true;
	}
	
	return (ia != a.end() || ib != b.end());
}

void reduce(const DFA& dfa)
{
	marks.clear();
	reduced_map.clear();
	reduced_states.clear();
	
	DFA::const_iterator d;
	for (d = dfa.begin(); d != dfa.end(); d++)
	{
		reduced_map[d->first] = reduced_states.size();
		reduced_states.push_back(d->first);
	}
	
	marks.resize(reduced_states.size());

	vector<DFA_State>::size_type i, j;
	for (i = 0; i < marks.size(); i++)
	{
		bool fi = is_final(reduced_states[i]);

		marks[i].resize(i);
		for (j = 0; j < i; j++)
		{
			bool fj = is_final(reduced_states[j]);
			marks[i][j] = (fi != fj);
		}
	}
	
	int count;
	do
	{
		count = 0;
		for (i = 0; i < marks.size(); i++)
		{
			DFA::const_iterator ti = dfa.find(reduced_states[i]);
			for (j = 0; j < i; j++)
			{
				if (marks[i][j]) continue;
				DFA::const_iterator tj = dfa.find(reduced_states[j]);
				if (!differ(ti->second, tj->second)) continue;
				count++;
				marks[i][j] = true;
			}
		}
	} while (count);
	
	for (i = 0; i < marks.size(); i++)
	{
		for (j = 0; j < i; j++)
		{
			if (!marks[i][j] && reduced_states[i].length())
			{
				reduced_map[reduced_states[i]] = j;
				reduced_states[i] = DFA_State();
			}
		}
	}
}

/************************************************************ Generate C */

enum Mode
{
	MODE_FIRST_HIT,
	MODE_LAST_HIT,
	MODE_TEST
};

Mode mode;

void dump_control(CState state, bool accept, bool precise, CState jump)
{
	switch (mode)
	{
	case MODE_TEST:
		if (precise && state == jump)
		{	/* Indefinite loop? */
			printf("\treturn %d;\n", accept?1:0);
			return;
		}
		
		printf("\tif (s == e) return %d;\n", accept?1:0);
		
		if (precise)
			printf("\t%s;\n\tgoto %s%u;\n", 
				scan_backwards?"e--":"s++", func, jump);
		break;
		
	case MODE_FIRST_HIT:
		if (accept)
		{
			printf("\treturn %s;\n",
				scan_backwards?"e":"s");
			return;
		}
		
		if (precise && state == jump)
		{
			printf("\treturn 0;\n");
			return;
		}
		
		printf("\tif (s == e) return 0;\n");
		if (precise)
			printf("\t%s;\n\tgoto %s%u;\n", 
				scan_backwards?"e--":"s++", func, jump);
		break;
	
	case MODE_LAST_HIT:
		if (accept)
			printf("\tlast = %s;\n",
				scan_backwards?"e":"s");
		
		if (precise && state == jump)
		{
			printf("\treturn last;\n");
			return;
		}
		
		printf("\tif (s == e) return last;\n");
		if (precise)
			printf("\t%s;\n\tgoto %s%u;\n", 
				scan_backwards?"e--":"s++", func, jump);
		break;
	}
}

void dump_case(CState state, CState* tbl)
{
	map<CState, basic_string<Input> >
		reverse;
	int	i;

	CState			biggest = 0;
	string::size_type	score;
	string::size_type	off;
	
	for (i = 0; i < 256; i++)
	{
		reverse[tbl[i]].push_back(i);
	}
	
	score = 0;
	map<CState, basic_string<Input> >::iterator j;
	for (j = reverse.begin(); j != reverse.end(); j++)
	{
		if (j->second.length() > score)
		{
			score = j->second.length();
			biggest = j->first;
		}
	}
	
	dump_control(state, is_final(reduced_states[state]),
		score == 256, biggest);
	
	/* No need for a switch */
	if (score == 256) return;
	
	printf("\tswitch (%s)\n\t{\n",
		scan_backwards?"*(--e)":"*s++");
	for (j = reverse.begin(); j != reverse.end(); j++)
	{
		if (j->first == biggest) continue;
		printf("\t");
		for (off = 0; off < j->second.length(); off++)
		{
			printf("case '\\x%02x': ", j->second[off]);
		}
		printf("\n\t\tgoto %s%u;\n", func, j->first);
	}
	printf("\tdefault: goto %s%u;\n\t}\n", func, biggest);
}

void compile(const NFA& nfa)
{
	fprintf(stderr, "Ok (%d NFA states)\nNFA->DFA algo..... ", nfa.size()); 
	fflush(stderr);
	
//	display(nfa);
	
	DFA_State start;
	DFA dfa = dedeterministic_it(nfa, start);
	
//	display(dfa);
	
	fprintf(stderr, "Ok (%d DFA states)\nState reduction... ", dfa.size());
	fflush(stderr);
	reduce(dfa);
	
	int k;
	CState i;
	CState go[256];
	CState nill = reduced_map[DFA_State()];
	
	CState states = 0;
	for (i = 0; i < reduced_states.size(); i++)
		if (i == nill || reduced_states[i].length())
			states++;
	fprintf(stderr, "Ok (%d DFA states)\nWriting source.... ", states);
	fflush(stderr);
	
	printf("%s %s(const char* s, const char* e)\n", 
		(mode==MODE_TEST)?"int":"const char*", func);
	printf("{\n");
	if (mode == MODE_LAST_HIT) printf("\tconst char* last = 0;\n");
	
	printf("\tgoto %s%u;\n\t\n", func, reduced_map[start]);
	
	for (i = 0; i < reduced_states.size(); i++)
	{
		if (i != nill && !reduced_states[i].length()) continue;
		
		printf("%s%u:\n", func, i);
		
		Transition& t = dfa[reduced_states[i]];
		Transition::iterator j;

		for (k = 0; k < 256; k++)
			go[k] = nill;
		
		for (j = t.begin(); j != t.end(); j++)
		{
			CState d = reduced_map[j->second];
			go[j->first] = d;
		}
		
		dump_case(i, go);
	}
	
	printf("}\n\n");
	fprintf(stderr, "Ok\n"); fflush(stderr);
}

int main(int argc, const char** argv)
{
	Input buf[1024];
	char  fnc[1024];
	
	if (argc != 4 || (strcmp(argv[1], "scan") && strcmp(argv[1], "match")))
	{
		fprintf(stderr, "Syntax: %s <match|scan> <func> <regex>\n", argv[0]);
		return 1;
	}
	
	printf("/* Meticulously hand-crafted regex scanner for: %s */\n", 
		argv[2]);
	
	if (!strcmp(argv[1], "match"))
	{
		func   = argv[2];
		regexp = (unsigned char*)argv[3];
		mode   = MODE_TEST;
		scan_backwards = false;
		
		fprintf(stderr, "Parsing regexp.... "); fflush(stderr);
		return yyparse();
	}
	else
	{
		snprintf(&fnc[0], sizeof(buf), "%s_pass1", argv[2]);
		snprintf((char*)&buf[0], sizeof(buf), "(%s).*", argv[3]);
		func   = &fnc[0];
		regexp = &buf[0];
		mode   = MODE_LAST_HIT;
		scan_backwards = true;
		
		fprintf(stderr, "Building pass1 scanner:\n===\n");
		fprintf(stderr, "Parsing regexp.... "); fflush(stderr);
		if (yyparse()) return 1;
		
		snprintf(&fnc[0], sizeof(buf), "%s_pass2", argv[2]);
		func   = &fnc[0];
		regexp = (unsigned char*)argv[3];
		mode   = MODE_LAST_HIT;
		scan_backwards = false;
		
		fprintf(stderr, "\nBuilding pass2 scanner:\n===\n");
		fprintf(stderr, "Parsing regexp.... "); fflush(stderr);
		if (yyparse()) return 1;
		
		func = argv[2];
		
		fprintf(stderr, "\nBuilding dispatcer... "); fflush(stderr);
		printf("void %s(const char** s, const char** e)\n", func);
		printf("{\n");
		printf("\t*s = %s_pass1(*s, *e);\n", func);
		printf("\tif (*s) *e = %s_pass2(*s, *e);\n", func);
		printf("}\n");
		fprintf(stderr, "Ok\n");

		return 0;
	}
}
