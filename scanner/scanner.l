%option yylineno

DIGIT [0-9]
HEXDIGIT [0-9a-fA-F]
CHAR [a-zA-Z]
WHITESPACE [ \r\n\t]

IDENTIFIER {CHAR}({CHAR}|{DIGIT})*
NUM_DEC {DIGIT}*
NUM_HEX ${HEXDIGIT}*
COMMENT \/\/.*
KEYWORD (fun|if|then|else|let|in|not|head|tail|and|end|isnum|islist|isfun)
LEXEM (;|=|\+|-|\*|\.|<|=|\(|\)|->)

%%

{KEYWORD}|{LEXEM} {
    printf("%s\n", yytext);
}

{IDENTIFIER} {
    printf("ident %s\n", yytext);
}

{NUM_DEC} {
    long int num = strtol(yytext, NULL, 10);
    printf("num %ld\n", num);
}

{NUM_HEX} {
    long int num = strtol(yytext + 1, NULL, 16);
    printf("num %ld\n", num);
}


{COMMENT} { /* ignore comments */

}

{WHITESPACE} { /* ignore whitespace */

}

. { /* unknown token, error */
    printf("Error: unknown token %s at line %d\n", yytext, yylineno);
    exit(1);
}


%%

/* user code */

main(argc, argv)
int argc;
char **argv;
{
    if(argc > 1)
        yyin = fopen(argv[1], "r");
    else
        yyin = stdin;

    yylex();

    return 0;
}

