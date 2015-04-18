%{
    #include "y.tab.h"

    #include <limits.h>
    #include <stdlib.h>

    int lineno = 1;
%}

%option yylineno

/* definitions */

DIGIT [0-9]
HEXDIGIT [0-9a-fA-F]
CHAR [a-zA-Z]
NEWLINE [\n]
WHITESPACE [ \r\n\t]

IDENTIFIER {CHAR}({CHAR}|{DIGIT})*
NUM_DEC {DIGIT}+
NUM_HEX ${HEXDIGIT}+
COMMENT \/\/.*
KEYWORD (fun|if|then|else|let|in|not|head|tail|and|end|isnum|islist|isfun)
LEXEM (;|=|\+|-|\*|\.|<|=|\(|\)|->)

%%


"fun"       return FUN;
"if"        return IF;
"then"      return THEN;
"else"      return ELSE;
"let"       return LET;
"in"        return IN;
"not"       return NOT;
"head"      return HEAD;
"tail"      return TAIL;
"and"       return AND;
"end"       return END;
"isnum"     return ISNUM;
"islist"    return ISLIST;
"isfun"     return ISFUN;

"->"        return ARROW;

{LEXEM} {
    return yytext[0];
}

{IDENTIFIER} {
    return IDENT;
}

{NUM_DEC} {
    long int num = strtol(yytext, NULL, 10);
    if ((num == LONG_MIN || num == LONG_MAX) && errno == ERANGE) {
        printf("Error: number literal too big\n");
        exit(1);
    }

    return NUM;
}

{NUM_HEX} {
    long int num = strtol(yytext + 1, NULL, 16);
    if ((num == LONG_MIN || num == LONG_MAX) && errno == ERANGE) {
        printf("Error: number literal too big\n");
        exit(1);
    }

    return NUM;
}


{COMMENT} { /* ignore comments */

}

{NEWLINE} {
    lineno++;
}

{WHITESPACE} { /* ignore whitespace */

}

. { /* unknown token, error */
    printf("Error: unknown token %s at line %d\n", yytext, yylineno);
    exit(1);
}
