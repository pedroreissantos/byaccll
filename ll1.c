#include "defs.h"

static char hard_stack[] =
	"#ifndef YYSTACKSIZE\n"
	"#define YYSTACKSIZE 200\n"
	"#endif\n"
	"#ifndef YYMAXSTACKSIZE\n"
	"#define YYMAXSTACKSIZE 100*YYSTACKSIZE\n"
	"#endif\n"
	"int yyerror();\n"
	"static YYSTYPE *stack, *yyvsp, *eos;\n"
	"YYSTYPE yyval, yylval;\n"
	"static int\n"
	"#ifdef __GNUC__\n"
	"__inline__\n"
	"#endif\n"
	"yygrow ()\n"
	"{\n"
	"\tstatic int sz = YYSTACKSIZE;\n"
	"\tif (stack == 0) {\n"
	"\t\tstack = yyvsp = (YYSTYPE*)malloc(sz*sizeof(YYSTYPE));\n"
	"\t\tif (stack == 0) {\n"
	"\t\t\tyyerror(\"no stack\");\n"
	"\t\t\treturn 0;\n"
	"\t\t}\n"
	"\t} else {\n"
	"\t\tunsigned len = yyvsp-stack;\n"
	"\t\tYYSTYPE *s;\n"
	"\t\tif (2*sz > YYMAXSTACKSIZE) {\n"
	"\t\t\tyyerror(\"exceeded maximum stack size\");\n"
	"\t\t\treturn 0;\n"
	"\t\t}\n"
	"\t\tsz *= 2;\n"
	"\t\ts = (YYSTYPE*)realloc(stack,sz*sizeof(YYSTYPE));\n"
	"\t\tif (s == 0) {\n"
	"\t\t\tyyerror(\"out of stack: no grow\");\n"
	"\t\t\treturn 0;\n"
	"\t\t}\n"
	"\t\tstack = s;\n"
	"\t\tyyvsp = stack + len;\n"
	"\t}\n"
	"\teos = stack + sz;\n"
	"\treturn 1;\n"
	"}\n"
	;

/* the follow are compute by the union of all the nonterminal symbol lookaheads */
static short *fol;
short *follow(int symbol)
{
    register int i;
    register action *p;

    if (!ISVAR(symbol)) return 0;

    for (i = 0; i < ntokens; i++) fol[i] = 0;
    for (i = 0; i < nstates; i++)
	for (p = parser[i]; p; p = p->next)
	    if (p->action_code == REDUCE &&
		p->suppressed == 0 &&
		rlhs[p->number] == symbol)
		fol[p->symbol] = 1;
    return fol;
}

/* return the symbol number (0..nsyms) given its ASCII(or y.tab.h token) number */
static int find_symbol(int i)
{
    register int j;

    for (j = 2; j < ntokens; ++j)
      if (i == symbol_value[j])
        return j;
    return -1;
}

static short errcnt;
static int compute_firsts(short *first, int rule, int mark)
{
    register int item, symb;
    register short *sp;
    int count = 0;

    for (item = rrhs[rule]; ritem[item] >= 0; item++) {
      symb = ritem[item];

      if (mark >= 0 && first[symb]) { /* the symbol is already a first: not LL(1) */
	/* printf("nullable recursion of '%s' on rule %d.\n", */
	printf("left factoring of '%s' required on rule %d.\n",
		symbol_name[symb], rule-2);
	errcnt++;
        return count;
      }
      else
	if (mark >= 0)
	  first[symb] = mark; /* mark with the rule number */
        else
	  first[symb]++; /* count the number of times it occurs */

      if (ISTOKEN(symb)) {
	count++;
        return count;
      }

      for (sp = derives[symb]; *sp >= 0; sp++)
	count += compute_firsts(first, *sp, mark);

      if (!nullable[symb]) return count;
    }

    return 0;
}

/*
 * use stack[3] = { symbol, action *, YYLVAL }
 * when a symbol is removed, all actions are executed
 *
 * actions are pushed into the stack (the first at the end)
 * struct action { struct action *next; int actno; };
 */

static void ll1table()
{
    register int i, j, k, l;
    register short *sp;
    int count = 0, conflict = 0;
    register FILE *f = code_file;
    short *fol, *first = NEW2(nsyms, short);
    short *confl = NEW2(nsyms, short);
    short ids;
    
    set_derives();
    set_nullable();
    output_stored_text();
    output_stype();
    output_defines();
    output_debug();
    fprintf(f,	hard_stack);
    /* output tables */
    for (j = 0; j < ntokens; ++j)
      fprintf(f,  "/* %d -> %s */\n", j, symbol_name[j]);
    fprintf(f, "/* %d -> %s */\n", start_symbol, symbol_name[start_symbol]);
    for (i = ntokens+1; i < nsyms; i++) {
      fprintf(f, "/* %d -> %s -> -%d */\n", i, symbol_name[i], i-ntokens);
    }
    for (ids = 0, i = 2; i < ntokens; ++i)
	if (is_C_identifier(symbol_name[i])) ids++;
    fprintf(f,  "#define LL_NTERM %d\n#define LL_TERMS %d\n/* MULTIBYTE %d */\n"
    		"static short entry[] = { 0,", nvars, ntokens, ids);
    /* tab: sets the column(token) or line(var) in the parse table */
    for (i = 1; i < 257+ids; i++)
        fprintf(f, "%d,", find_symbol(i));
    fprintf(f,  " -1 };\n");
    fprintf(f,	"/* rules containing symbols in reverse order, ending in a 0 */"
    		"\nstatic short rule0[] = { 0 }; /* generic empty rule */\n");

    for (k = 4, i = 3; i < nrules; ++i, ++k) {
      fprintf(f,  "static short rule%d[] = { ", i-2);
      j = k;
      while (ritem[k] >= 0) k++;
      l = k;
      while (--l >= j)
	fprintf(f,  "%d, ", ISVAR(ritem[l]) ? ntokens-ritem[l] : ritem[l]);
      fprintf(f,  "0 };\n");
    }
    fprintf(f,	"static short *rules[] = {\n");
    for (i = 2; i < nrules; ++i)
      fprintf(f,  "\trule%i,\n", i-2);
    fprintf(f,  "0 };\n");

    fprintf(f,  "static short rulesize[] = { 1");
    for (k = 4, i = 3; i < nrules; ++i, ++k) {
      for (j = 0; ritem[k] >= 0; j++) k++;
      fprintf(f,  ", %d", j);
    }
    fprintf(f,  " };\n");

    fprintf(f,  "static short table[%d][%d] = {\n", nvars, ntokens+1);
    fprintf(f, "/*");
    for (k = 0; k < ntokens; k++)
      fprintf(f, "\t%s", symbol_name[k]);
    fprintf(f, " */\n");
    for (i = ntokens+1; i < nsyms; i++) {
      fol = 0;
      for (j = 0; j < nsyms; j++) confl[j] = first[j] = 0;
      count = 0;
      for (sp = derives[i]; *sp >= 0; sp++) {
	compute_firsts(confl, *sp, -1);
	if (compute_firsts(first, *sp, *sp) == 0)
	  count = *sp;
      }
      if (nullable[i])
        fol = follow(i);

      if (vflag > 1) {
	for (j = k = 0; j < nsyms; j++) if (first[j]) k++;
	fprintf(verbose_file, "First(%s) #%d =", symbol_name[i], k);
	for (k = 0; k < nsyms; k++)
	  if (first[k]) fprintf(verbose_file, " %s(rule %d),", symbol_name[k], first[k]-2);
	if (nullable[i])
	  fprintf(verbose_file, " \\empty(rule %d)", count-2);
	fprintf(verbose_file, "\n");
      }

      fprintf(f, "/* %s */ {", symbol_name[i]);
      for (k = 0; k < ntokens; k++) {
	if (confl[k] > 1 || (first[k] && fol && fol[k])) {
	  fprintf(f, "\t(%d),", first[k]-2);
	  /* should use a value <-1 as an index into a LL(k) dfa (or switch) function that
	   * returns the rule number (be careful not to move the input pointer) */
	  conflict++;
	}
	else
	  if (first[k]) fprintf(f, "\t%d,", first[k]-2);
	  else
	    if (fol && fol[k]) fprintf(f, "\t%d,", count-2);
	    else fprintf(f, "\t-1,");
      }
      fprintf(f, " 0 },\n");
    }
    fprintf(f,  "0 };\n\n");
    fprintf(f,  "static void pp(int *pilha, int top) {\n"
    		"  int i; printf(\"%%d#\", top+1);\n"
      		"  for (i = 0; i <= top; i++) printf( \" %%d\", pilha[i]);\n"
        	"  printf(\"\\n\");\n}\n\n");
    fprintf(f,	"int yydebug;\nstatic void ll_action(int yyn) {\n"
		"#if YYDEBUG\n"
		// "  YYSTYPE *s = stack; printf(\"#%%d: \", yyvsp-s); while (s < yyvsp) printf(\" %%d\", s->i), s++; printf(\"\\n\");\n"
		"  if (yydebug) printf(\"conclui regra %%d %%s.\\n\", yyn, yyrule[yyn]);\n"
		"#endif\n"
    		"  yyvsp--; switch(yyn) {\n");
    output_semantic_actions();
    fprintf(f,	"  default: break; }\nyyvsp++; }\n");
    /* output analyser: from pp.120 e 237, Fisher & LeBlanc */
    fprintf(f,  "int yyparse() {\n"
		"  int yychar, yyn;\n  short *yyrhs;\n"
		"  int pilha[1000];\n"
		"  int top = -1;\n"
		"#define TOP pilha[top]\n"
		"#define PUSH pilha[++top]\n"
		"#define POP pilha[top--]\n"
		"  PUSH = -%d; /* push start symbol */\n"
		"  yychar = yylex();\n"
		"#if YYDEBUG\n"
		"  if (yydebug) printf(\"reading %%d (%%d)\\n\", yychar, entry[yychar]);\n"
		"#endif\n"
		"  while (top >= 0) {\n"
		// "    pp(pilha, top);\n"
		"    if (TOP < 0 && TOP > -%d) { /* is nonterminal */\n"
		"      /* lookup terminal symbol in table and replace it by its production */\n"
		"      if ((yyn = table[-TOP-1][entry[yychar]]) == -1) return yyerror(\"Syntax Error.\"), 1;\n"
		"#if YYDEBUG\n"
		"      if (yydebug) printf(\"selecting rule %%d (%%s)\\n\", yyn, yyrule[yyn]);\n"
		"#endif\n"
		"      POP;\n"
		"      PUSH =-(%d+yyn);\n"
		"      for (yyrhs = rules[yyn]; yyrhs[0] != 0; yyrhs++)\n"
		"        PUSH = yyrhs[0];\n"
		"    } else if (TOP > 0) {\n"
		"      if (TOP == entry[yychar]) {\n"
		"        if (yyvsp + 1 >= eos) yygrow();\n"
		"        *yyvsp++ = yylval;\n"
		"        POP; /* remove the yychar that was read...  ...and load a new one */\n"
		"        yychar = yylex();\n"
		"printf(\".+1\\n\");\n"
		"#if YYDEBUG\n"
		"        if (yydebug) printf(\"reading %%d (%%d)\\n\", yychar, entry[yychar]);\n"
		"#endif\n"
		"      } else return yyerror(\"Syntax Error.\"), 1; /* terminal symbol not matched */\n"
		"    } else { /* top is a semantic action */\n"
		"      if (yyvsp + 1 >= eos) yygrow();\n"
		"      ll_action(-TOP-%d);\n"
		"      yyvsp -= rulesize[-TOP-%d];\n"
		"      *yyvsp++ = yyval;\n"
		"      POP;\n"
		"    }\n"
		"  }\n"
		"}\n", ritem[1]-ntokens, nrules, nrules, nrules, nrules
    	   );
    output_trailing_text();
    if (vflag > 1) {
	for (i = start_symbol+1; i < nsyms; i++) {
	    fol = follow(i);
	    fprintf(verbose_file, "Follow(%s) = ", symbol_name[i]);
	    for (j = 0; j < ntokens; j++)
		if (fol[j])
		    fprintf(verbose_file, " %s", symbol_name[j]);
	    fprintf(verbose_file, "\n");
	}
    }
    done(0);
}

void ll1hard()
{
    register int i, j, k, len;
    register FILE *f = code_file;
    short *look, *first, target, *sp;
    int null;
    
    errcnt = 0;

    fprintf(f,	"#include <ctype.h>\n#include <stdio.h>\n");
    set_derives();
    set_nullable();
    output_stored_text();
    output_stype();
    output_defines();
    output_debug();
    fprintf(f,	hard_stack);
    fprintf(f,	"int yydebug;\nstatic void ll_action(int yyn) {\n"
		"#if YYDEBUG\n"
		// "  YYSTYPE *s = stack; while (s < yyvsp) printf(\" %%d\", s->i), s++; printf(\"\\n\");\n"
		"  if (yydebug) printf(\"rule %%d concluded: %%s.\\n\", yyn, yyrule[yyn]);\n"
		"#endif\n"
    		"  yyvsp--; switch(yyn) {\n");
    output_semantic_actions();
    fprintf(f,	"  default: break; }\nyyvsp++; }\n");

    /* look: lookahead symbols for each rule (and non-terminal)
             look[i] is 0 if symbol_name[i] is not a lookahead
	     look[i] is k where k is the rule that produced the lookahead */
    look = NEW2(nsyms, short);
    /* first: all first terminal symbols of a production (rule) */
    first = NEW2(nsyms, short);

    for (i = 3; i < nrules; ++i)
        if (rlhs[i] != rlhs[i-1])
            fprintf(f, "static int ll_%s();\n", symbol_name[rlhs[i]]);
    fprintf(f,	"static int (*input)(void*);\nstatic void *data;\n"
    		"static int yychar;\nextern int yyerror(char*), yylex();\n"
		"#define error YYERRCODE\n"
		"extern void exit(int);\nint yynerrs;\n"
		"static int ll_error() {\n  yynerrs++;\n"
		"  yyerror(\"syntax error\"); return 1; }\n"
    		"int ll_parse(int (*inp)(void*), void *ind) {\n"
    		"  input = inp; data = ind; yychar = (*input)(data);\n"
		"  ll_%s();\n  if (yychar) ll_error();\n  return yynerrs; }\n"
		"int yyparse() { return ll_parse(yylex, 0); }\n",
		symbol_name[ritem[1]]);

    null = 0;
    for (i = 3; i < nrules; ++i) {
        if (rlhs[i] != rlhs[i-1]) {
	  if (i != 3) {
	    if (nullable[rlhs[i-1]]) {
	      if (null == 0) fprintf(stderr, "internal error: unknown nullable for rule %d\n", i-3);
	      fprintf(f, "  ll_action(%d);\n  return 0;\n", null-2);
#if 0
fprintf(f, "  /* null = %d */ ll_action(%d);\n  return 0;\n}\n", null-2, i-3);
#endif
	    }
	    else
	      fprintf(f, "  return ll_error();\n");
	    null = 0;
	    fprintf(f, "}\n\n");
	  }
	  fprintf(f, "static int ll_%s()\n{\n", symbol_name[rlhs[i]]);
	  for (j = 0; j < nsyms; j++) look[j] = 0;
	  target = rlhs[i];
	  for (j = 0; j < nsyms; j++) first[j] = 0;
	  for (sp = derives[target]; *sp >= 0; sp++)
	    compute_firsts(first, *sp, *sp); /* grammar is factorized? */
	}
	if (ritem[rrhs[i]] < 0) { /* empty rule */
#ifdef DEBUG
	printf("rule %d is empty.\n", i-2);
#endif
	  if (null) {
	    printf("more than one nullable rule: %d and %d.\n", null, i);
	    errcnt++;
	  }
	  null = i;
	  continue;
	}
	if (ritem[rrhs[i]] == target) {
	  printf("left recursion of symbol '%s' on rule %d.\n",
	         symbol_name[ritem[rrhs[i]]], i);
	  errcnt++;
	}
	if (ISVAR(ritem[rrhs[i]])) {
	  if (look[ritem[rrhs[i]]] != 0) {
	    printf("left factoring of non-terminal '%s' on rule %d.\n",
	           symbol_name[ritem[rrhs[i]]], i);
	    errcnt++;
	  }
	  else
	    look[ritem[rrhs[i]]] = i;
	}
	/* else ISTOKEN() printf(">>%s\n", symbol_name[ritem[rrhs[i]]], i); */

	for (j = 0; j < nsyms; j++) first[j] = 0;
	if (compute_firsts(first, i, 1) == 0) {
	  if (null) {
	    printf("more than one nullable rule: %d and %d.\n", null, i);
	    errcnt++;
	  }
	  null = i;
	}
#ifdef DEBUG
	printf("rule %d has firsts:", i-2);
	for (j = 0; j < nsyms; j++)
	  if (first[j])
	    printf(" %s", symbol_name[j]);
	printf("\n");
#endif
	fprintf(f, "  if (");
	for (k = j = 0; ISTOKEN(j); j++)
	  if (first[j])
	    fprintf(f, "%syychar == %s", k++ ? " || " : "", symbol_name[j]);
	if (k == 0) fprintf(f, "1");
	fprintf(f, ") {\n"
		"#if YYDEBUG\n"
		"\tif (yydebug) printf(\"select rule %d with symbol "
		"%%d (%%s).\\n\", yychar, yyname[yychar]);\n#endif\n", i-2);
        for (len = 0, j = rrhs[i]; ritem[j] >= 0; ++j, len++)
	  ;
	fprintf(f, "\tif (yyvsp + %d >= eos) yygrow();\n", len);
        for (k = 0, j = rrhs[i]; ritem[j] >= 0; ++j, k++) {
	  if (ISTOKEN(ritem[j])) {
	    if (k != 0)
	      fprintf(f, "\tif (yychar != %s) return ll_error();\n",
	      		symbol_name[ritem[j]]);
	    fprintf(f, "\tyychar = (*input)(data);\n\t*yyvsp++ = yylval;\n");
	  }
	  else fprintf(f, "\tif (ll_%s()) return 1;\n\t*yyvsp++ = yyval;\n",
	  		  symbol_name[ritem[j]]);
	}
	fprintf(f, "\tll_action(%d);\n\tyyvsp -= %d;\n\treturn 0;\n  }\n",
		   i-2, len);
    }
    if (nullable[rlhs[i-1]]) {
      if (null == 0) fprintf(stderr, "internal error: unknown nullable for rule %d\n", i-3);
      fprintf(f, "  ll_action(%d);\n  return 0;\n}\n", null-2);
    }
#if 0
fprintf(f, "  /* null = %d */ ll_action(%d);\n  return 0;\n}\n", null-2, i-3);
#endif
    else
      fprintf(f, "  ll_error();\n}\n");

    if (errcnt)
      fprintf(stderr, "Grammar is not LL(1): (%d %s)\n", errcnt,
      		errcnt > 1 ? "errors" : "error");

    output_trailing_text();
    done(0);
}

void ll1()
{
    fol = NEW2(ntokens,short);
    if (pflag)
	if (hflag)
	  ll1hard();
	else
	  ll1table();
}
