#include <stdio.h>
#include <stdlib.h>
/* header for the parser combinator - for creating our grammer */
#include "mpc.h"

/* if we are using Windows */
#ifdef _WIN32
#include <string.h>

/* declare an array for the user input */
static char buffer[2048];

/* create a windows version of the readline function */
char* readline(char* prompt) {
    fputs(prompt, stdout);
    fgets(buffer, 2048, stdin);
    char * cpy = malloc(strlen(buffer)+1);
    strcpy(cpy, buffer);
    cpy[strlen(cpy)-1] = '\0';
    return cpy;
}

/* windows add_history function */

void add_history(char* unused) {}

/* if we are not using windows, use editline headers */
#else
#include <editline/readline.h>
#include <editline/history.h>
#endif


int main(int argc, char** argv) {
    
    /* set up our grammer */
    /*create our parsers */
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Lispy = mpc_new("lispy");
    
    /* define the language */
    mpca_lang(MPCA_LANG_DEFAULT,
        "                                                           \
            number      : /-?[0-9]+/ ;                              \
            operator    : '+' | '-' | '*' | '/' ;                   \
            expr        : <number> | '(' <operator> <expr>+ ')' ;   \
            lispy       : /^/ <operator> <expr>+ /$/ ;              \
        ",
        Number, Operator, Expr, Lispy);

    /* Print version and exit info */
    puts("Lispy Version 0.0.0.1");
    puts("Press Ctrl+c to exit\n");
    
    /* In a never ending loop */
    while(1) {
        
        /* now the readline function will work on both operating systems*/
        char* input = readline("lispy> ");
        add_history(input);
        
        /* parse input */
        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lispy, &r)) {
            /*parse successful */
            mpc_ast_print(r.output);
            mpc_ast_delete(r.output);
        } else {
            /* Parse unsuccessful */
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }
        
        
        free(input);
    }
    
    /* undefine and delete our parsers */
    mpc_cleanup(4, Number, Operator, Expr, Lispy);
    
    return 0;
}