#include "defs.h"
/* rules whose name begins with '$' are ignored by the lexical analyser (discarded)
 * symbols enclose in '"' are considered regular expressions (returning the token value)
 * symbols enclose in '\'' are considered single characters (returning the ASCII code)
 */
void lexical() {
	if (xflag) {
		int i;
		printf("lexical:");
		for (i = 0; i < ntokens; i++)
			if (symbol_name[i][0] == '"' || symbol_name[i][0] == '\'')
				printf(" %s=%d", symbol_name[i], symbol_value[i]);
		printf("\n");
		for (i = 3; i < nrules; ++i)
			if (symbol_name[rlhs[i]][0] == '$') {
				int j;
				printf("[%d->%d]=%s\n", i, rlhs[i], symbol_name[rlhs[i]]);
				for (j = rrhs[i]; ritem[j] > 0; j++)
					printf("\t%s\n", symbol_name[ritem[j]]);
				printf("\treduce=%d\n", -ritem[j]);
			}
	}
}
