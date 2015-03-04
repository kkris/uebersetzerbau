/* definitions */

DIGIT [0-9]
HEXDIGIT [0-9a-fA-F]
HEXDIGITNONNULL [1-9a-fA-F]
CHAR [a-zA-Z]

%%

fun|if|then|else|let|in|not|head|tail|and|end|isnum|islist|isfun { /* match keywords */
    printf("%s\n", yytext);
}

;|=|\+|-|\*|\.|<|=|\(|\)|-> {
    printf("%s\n", yytext);
}

{CHAR}({CHAR}|{DIGIT})* {
    printf("ident %s\n", yytext);
}

{DIGIT}* {
    char *end;
    long int num = strtol(yytext, &end, 10);
    /* TODO: validate end */

    printf("num %ld\n", num);
}

${HEXDIGITNONNULL}{HEXDIGIT}* {
    char *end;

    /* skip $ */
    char *token = yytext;
    token++;

    long int num = strtol(token, &end, 16);
    /* TODO: validate end */

    printf("num %ld\n", num);
}


\/\/.*$ { /* ignore comments */

}

[ \r\n\t] { /* ignore whitespace */

}


%%

/* user code */

main(argc, argv)
int argc;
char **argv;
{
    ++argv, --argc;
    if(argc > 0)
        yyin = fopen(argv[0], "r");
    else
        yyin = stdin;

    yylex();
}

