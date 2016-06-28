#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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

/* enum of possible error types */
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };
/* enum of possible lval types */
enum { LVAL_NUM, LVAL_ERR };

/* declare lval structure*/
typedef struct {
    int type;
    long num;
    int err;
} lval;

/* declare lval methods */
lval lval_num(long x);
lval lval_err(int x);
void lval_print(lval v);
void lval_println(lval v);

/*declare eval methods */
lval eval(mpc_ast_t* t);
lval eval_op(lval x, char* op, lval y);




int main(int argc, char** argv) {
    
    /* set up our grammer */
    /*create our parsers */
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Lispy = mpc_new("lispy");
    
    /* define the language */
    mpca_lang(MPCA_LANG_DEFAULT,
        "                                                                       \
            number      : /-?[0-9]+/ ;                                          \
            operator    : '+' | '-' | '*' | '/' | '%' | '^' | /min/ | /max/ ;   \
            expr        : <number> | '(' <operator> <expr>+ ')' ;               \
            lispy       : /^/ <operator> <expr>+ /$/ ;                          \
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
            
            lval result = eval(r.output);
            lval_println(result);
            
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



/* create a new number type lval */
lval lval_num(long x) {
    lval v;
    v.type = LVAL_NUM;
    v.num = x;
    return v;
}

/* create a new error type lval */
lval lval_err(int x) {
    lval v;
    v.type = LVAL_ERR;
    v.err = x;
    return v;
}

/* print an "lval" */
void lval_print(lval v) {
    switch (v.type) {
        /* if it is a number */
        case LVAL_NUM: 
            printf("%li", v.num); 
            break;
            
        /* if it is an error */
        case LVAL_ERR:
            /* check error type and print error */
            if (v.err == LERR_DIV_ZERO) {
                printf("Error: Division by zero!");
            }
            if(v.err == LERR_BAD_OP) {
                printf("Error: Invalid Operator!");
            }
            if(v.err == LERR_BAD_NUM) {
                printf("Error: Invalid Number!");
            }
            break;
    }
}

/* print an "lval" followed by a newline */
void lval_println(lval v) {
    lval_print(v);
    putchar('\n');
}

/* evaluates our parsed code */ 
lval eval(mpc_ast_t* t) {
    
    /* if tagged as number return it directly */
    if(strstr(t->tag, "number")) {
        /* check for conversion errors */
        errno = 0;
        long x = strtol(t->contents, NULL, 10);
        return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
    }
    
    /* the operator is always the second child */
    char* op = t->children[1]->contents;
    
    /* store the value of the first expression in variable x */
    lval x = eval(t->children[2]);
    
    /* iterate through the remaining children and accumulate the total(x) */
    int i = 3;
    while(strstr(t->children[i]->tag, "expr")) {
        x = eval_op(x, op, eval(t->children[i]));
        i++;
    }
    
    return x;
}

/* calculate a mathematical result based on an operator passed as an argument */
lval eval_op(lval x, char* op, lval y) {
    if(strcmp(op, "+") == 0) { return lval_num(x.num + y.num); }
    if(strcmp(op, "-") == 0) { return lval_num(x.num - y.num); }
    if(strcmp(op, "*") == 0) { return lval_num(x.num * y.num); }
    if(strcmp(op, "/") == 0) {
        return y.num == 0
            ? lval_err(LERR_DIV_ZERO)
            : lval_num(x.num / y.num); 
    }
    if(strcmp(op, "%") == 0) {
        return y.num == 0
            ? lval_err(LERR_DIV_ZERO)
            : lval_num(x.num % y.num); 
    }
    if(strcmp(op, "^") == 0) { return lval_num((long) pow(x.num, y.num)); }
    if(strcmp(op, "min") == 0) {
        if(x.num > y.num) {
            return lval_num(y.num);
        } else {
            return lval_num(x.num);
        }
    }
    if(strcmp(op, "max") == 0) {
        if(x.num < y.num) {
            return lval_num(y.num);
        } else {
            return lval_num(x.num);
        }
    }
    return lval_err(LERR_BAD_OP);
}

