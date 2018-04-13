#include "defs.h"

static char hard_init[] =
	"#define YYABORT return -1\n"
	"#define YYACCEPT return 0\n"
	"#define yyclearin if ((yychar = yylex()) < 0) yychar = 0; else\n"
	"#define yyerrorok yyerrflag = 3\n"
	"#define YYERROR goto user_error_handler\n"
	"#define YYRECOVERING() (yyerrflag <= 2)\n"
	"int yydebug, yylex();\n"
	"YYSTYPE yylval, yyval;\n"
	"int yyparse() {\n"
	"\tint yychar;\n"
	"\tunsigned yyerrflag = 3;\n"
	"\tyygrow();\n"
	"\tif ((yychar = yylex()) < 0) yychar = 0; else\n"
	"\tgoto state_0;\n"
	;
static char hard_end[] =
	"\tYYABORT; /* just in case ... */\n"
	"}\n"
	;

static char hard_stack[] =
	"#ifndef YYSTACKSIZE\n"
	"#define YYSTACKSIZE 200\n"
	"#endif\n"
	"#ifndef YYMAXSTACKSIZE\n"
	"#define YYMAXSTACKSIZE 100*YYSTACKSIZE\n"
	"#endif\n"
	"int yyerror();\n"
	"typedef struct { int state; YYSTYPE val; } StackType;\n"
	"static StackType *stack, *tos, *eos;\n"
	"static int\n"
	"#ifdef __GNUC__\n"
	"__inline__\n"
	"#endif\n"
	"yygrow ()\n"
	"{\n"
	"\tstatic int sz = YYSTACKSIZE;\n"
	"\tif (stack == 0) {\n"
	"\t\tstack = tos = (StackType*)malloc(sz*sizeof(StackType));\n"
	"\t\tif (stack == 0) {\n"
	"\t\t\tyyerror(\"no stack\");\n"
	"\t\t\treturn 0;\n"
	"\t\t}\n"
	"\t} else {\n"
	"\t\tunsigned len = tos-stack;\n"
	"\t\tStackType *s;\n"
	"\t\tif (2*sz > YYMAXSTACKSIZE) {\n"
	"\t\t\tyyerror(\"exceeded maximum stack size\");\n"
	"\t\t\treturn 0;\n"
	"\t\t}\n"
	"\t\tsz *= 2;\n"
	"\t\ts = (StackType*)realloc(stack,sz*sizeof(StackType));\n"
	"\t\tif (s == 0) {\n"
	"\t\t\tyyerror(\"out of stack: no grow\");\n"
	"\t\t\treturn 0;\n"
	"\t\t}\n"
	"\t\tstack = s;\n"
	"\t\ttos = stack + len;\n"
	"\t}\n"
	"\teos = stack + sz;\n"
	"\treturn 1;\n"
	"}\n"
	;
static char hard_stack_old[] =
	"    int old_stacksize = yystacksize;\n"
	"    short *new_yyss;\n"
	"    YYSTYPE *new_yyvs;\n"
	"\n"
	"    if (yystacksize == YYMAXSTACKSIZE)\n"
	"        return (1);\n"
	"    yystacksize += (yystacksize + 1 ) / 2;\n"
	"    if (yystacksize > YYMAXSTACKSIZE)\n"
	"        yystacksize = YYMAXSTACKSIZE;\n"
	"#if YYDEBUG\n"
	"    if (yydebug)\n"
	"        printf(\"yydebug: growing stack size from %d to %d\n\",\n"
	"               old_stacksize, yystacksize);\n"
	"#endif\n"
	"    new_yyss = (short *) yyrealloc ((char *)yyss, yystacksize * sizeof (short));\n"
	"    if (new_yyss == 0)\n"
	"        return (1);\n"
	"    new_yyvs = (YYSTYPE *) yyrealloc ((char *)yyvs, yystacksize * sizeof (YYSTYP\n"
	"E));\n"
	"    if (new_yyvs == 0)\n"
	"    {\n"
	"        yyfree (new_yyss);\n"
	"        return (1);\n"
	"    }\n"
	"    yyss = new_yyss;\n"
	"    yyvs = new_yyvs;\n"
	"    return (0);\n"
	"}\n"
	;
static char hard_error1[] =
	"error_handler:\n"
	"\tif (yyerrflag > 2) yyerror(\"syntax error\");\n"
	"user_error_handler:\n"
	"\tif (yyerrflag == 0) {\n"
	"\t\tif (yychar == 0) YYABORT;\n"
	"\t\tif ((yychar = yylex()) < 0) yychar = 0;\n"
	"\t\tswitch ((tos-1)->state) {\n"
	;
static char hard_error2[] =
	"\t\t}\n"
	"\t} else {\n"
	"\t\tyyerrflag = 0;\n"
	"\t\twhile (tos != stack) {\n"
	"\t\t\tswitch ((tos-1)->state) {\n"
	;
static char hard_error3[] =
	"\t\t\t\tdefault: break;\n"
	"\t\t\t}\n"
	"\t\t\ttos--;\n"
	"\t\t}\n"
	"\t\tYYABORT;\n"
	"\t}\n"
	;

static char *shift_state;

static void
hard_gotos()
{
    register int i, k, as, symb;
    register short *to_state;
    register shifts *sp;

    for (symb = start_symbol+1; symb < nsyms; symb++) {
	fprintf(code_file, "nonterminal_%s:\n\tswitch ((tos-1)->state) {\n",
		symbol_name[symb]);
	for (sp = first_shift; sp; sp = sp->next) {
	    to_state = sp->shift;
	    for (i = 0; i < sp->nshifts; ++i)
	    {
		k = to_state[i];
		as = accessing_symbol[k];
		if (ISVAR(as) && as == symb)
		    fprintf(code_file, "\t\tcase %d: goto state_%d;\n",
			    sp->number, k);
	    }
	}
	fprintf(code_file, "\t}\n");
    }
}

void
shiftstates()
{
    register int count, i;
    register action *p, *q;

    shift_state = NEW2(nstates,char);
    count = 0;
    for (i = 0; i < nstates; i++) {
	p = parser[i];
	for (q = p; q; q = q->next)
	    if (q->suppressed < 2 && q->action_code == SHIFT)
		++count;

	if (count > 0)
	    for (; p; p = p->next)
		if (p->action_code == SHIFT && p->suppressed == 0)
		      shift_state[p->number] = 1;
    }
}

static void
hard_actions(stateno)
int stateno;
{
    register action *p, *q;
    register shifts *sp;
    register int as, k, anyreds, count;

    if (stateno == final_state)
	fprintf(code_file, "\t\tcase 0: YYACCEPT;\n");

    p = parser[stateno];
    if (p) {
	anyreds = count = 0;
	for (q = p; q; q = q->next) {
	    if (q->suppressed < 2 && q->action_code == SHIFT)
		++count;
	    if (q->action_code == REDUCE && q->suppressed < 2)
		anyreds = 1;
	}

	for (; p; p = p->next) {
	    if (count && p->action_code == SHIFT && p->suppressed == 0 &&
	    	p->symbol > 1)
		fprintf(code_file, "\t\tcase %s: goto state_%d; /* s%d */\n",
			    symbol_name[p->symbol], p->number, p->number);
	    if (anyreds && p->action_code == REDUCE && p->number != defred[stateno])
	    {
		k = p->number - 2;
		if (p->suppressed == 0)
		    fprintf(code_file, "\t\tcase %s: goto reduce_%d;\n",
			    symbol_name[p->symbol], k);
	    }
	}
	if (defred[stateno] > 0)
	    fprintf(code_file, "\t\tdefault: goto reduce_%d;\n", defred[stateno] - 2);
	else
	    fprintf(code_file, "\t\tdefault: goto error_handler;\n");
    }
}

static void
hard_state(state)
int state;
{
    fprintf(code_file, "state_%d: tos->state = %d;\n", state, state);
    if (shift_state[state])
      fprintf(code_file, "\ttos->val = yylval;\n"
      			 "\tif ((yychar = yylex()) < 0) yychar = 0;\n"
			 "\tyyerrflag++;\n");
    fprintf(code_file,   "\tif (++tos == eos && yygrow() == 0) YYABORT;\n"
    			 "state_action_%d:\n"
			 "\tswitch (yychar) {\n", state);
    hard_actions(state);
    fprintf(code_file, "\t}\n\n");
}

static void
hard_error()
{
    register int i;
    register action *p, *q;
    register int stateno, count;

    fprintf(code_file, hard_error1);
    for (i = 0; i < nstates; i++)
        fprintf(code_file, "\t\t\tcase %d: goto state_action_%d;\n", i, i);
    fprintf(code_file, hard_error2);

    for (stateno = 0; stateno < nstates; stateno++) {
	p = parser[stateno];
	if (p) {
	    count = 0;
	    for (q = p; q; q = q->next)
		if (q->suppressed < 2 && q->action_code == SHIFT)
		    ++count;

	    for (; p; p = p->next)
		if (count && p->action_code == SHIFT && p->suppressed == 0 &&
		    p->symbol == 1)
		    fprintf(code_file, "\t\t\t\tcase %d: goto state_%d;\n",
				stateno, p->number);
	}
    }
    fprintf(code_file, hard_error3);
}

void
hardcode()
{
    register int i;

    if (!hflag) return;

    shiftstates();
    output_stored_text();
    output_stype();
    output_defines();
    output_debug();
    fprintf(code_file, hard_stack);
    fprintf(code_file, hard_init);

    for (i = 0; i < nstates; i++)
	hard_state(i);

    hard_gotos();
    output_semantic_actions();
    hard_error();
    fprintf(code_file, hard_end);
    output_trailing_text();
}
