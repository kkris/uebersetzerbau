%{
    #include "symbol.h"
    #include "y.tab.h"
%}

%option yylineno

/* definitions */

DIGIT [0-9]
HEXDIGIT [0-9a-fA-F]
CHAR [a-zA-Z]
WHITESPACE [ \r\n\t]

IDENTIFIER {CHAR}({CHAR}|{DIGIT})*
NUM_DEC {DIGIT}*
NUM_HEX ${HEXDIGIT}*
COMMENT \/\/.*
KEYWORD (fun|if|then|else|let|in|not|head|tail|and|end|isnum|islist|isfun)
LEXEM (;|=|\+|-|\*|\.|<|\(|\)|->)

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


";"         return ';';
"="         return '=';
"+"         return '+';
"-"         return '-';
"*"         return '*';
"."         return '.';
"<"         return '<';
"("         return '(';
")"         return ')';

{IDENTIFIER} {
    return IDENT; @{ @IDENT.name@ = strdup(yytext); @}
}

{NUM_DEC} {
    long int num = strtol(yytext, NULL, 10);
    return NUM; @{ @NUM.value@ = num; @}
}

{NUM_HEX} {
    long int num = strtol(yytext + 1, NULL, 16);
    return NUM; @{ @NUM.value@ = num; @}
}


{COMMENT} { /* ignore comments */

}

{WHITESPACE} { /* ignore whitespace */

}

. { /* unknown token, error */
    printf("Error: unknown token %s at line %d\n", yytext, yylineno);
    exit(1);
}
