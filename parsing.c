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

#define LASSERT(args, cond, err) \
    if(!(cond)) { lval_del(args); return lval_err(err); }
    
/* forward declare types */
struct lval;
struct lenv;
typedef struct lval lval;
typedef struct lenv lenv;

/* enum of possible lval types */
enum { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR,
    LVAL_FUN, LVAL_QEXPR };
    
typedef lval*(*lbuiltin)(lenv*, lval*);

/* declare lval structure*/
struct lval{
    int type;
    long num;
    /* Error and Symbol types have some string data*/
    char* err;
    char* sym;
    lbuiltin fun;
    /* count and pointer to a list of "lval*" */
    int count;
    lval** cell;
};

/* declare lenv structure */
struct lenv {
    int count;
    char** syms;
    lval** vals;
};

/* declare lval methods */
lval* lval_num(long x);
lval* lval_err(char* m);
lval* lval_sym(char* s);
lval* lval_sexpr(void);
lval* lval_qexpr(void);
lval* lval_fun(lbuiltin func);
lval* lval_read_num(mpc_ast_t* t);
lval* lval_read(mpc_ast_t* t);
lval* lval_add(lval* v, lval* x);
lval* lval_pop(lval* v, int i);
lval* lval_take(lval* v, int i);
lval* lval_join(lval* x, lval* y);
lval* lval_copy(lval* v);
void lval_del(lval* v);
void lval_print(lval* v);
void lval_println(lval* v);
void lval_expr_print(lval* v, char open, char close);

/*declare lenv methods */
lenv* lenv_new(void);
void lenv_del(lenv* e);

/* declare eval methods - (bodies are directly after main) */
lval* lval_eval_sexpr(lval* v);
lval* lval_eval(lval* v);
lval* builtin(lval* a, char* func);
lval* builtin_op(lval* a, char* op);
lval* builtin_head(lval* a);
lval* builtin_tail(lval* a);
lval* builtin_list(lval* a);
lval* builtin_eval(lval* a);
lval* builtin_join(lval* a);


int main(int argc, char** argv) {
    
    /* set up our grammer */
    /*create our parsers */
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Symbol = mpc_new("symbol");
    mpc_parser_t* Sexpr = mpc_new("sexpr");
    mpc_parser_t* Qexpr = mpc_new("qexpr");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Lispy = mpc_new("lispy");
    
    /* define the language */
    mpca_lang(MPCA_LANG_DEFAULT,
        "                                                                           \
            number      : /-?[0-9]+/ ;                                              \
            symbol      : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;                        \
            sexpr       : '(' <expr>* ')' ;                                         \
            qexpr       : '{' <expr>* '}' ;                                         \
            expr        : <number> | <symbol> | <sexpr> | <qexpr> ;                 \
            lispy       : /^/ <expr>* /$/ ;                                         \
        ",
        Number, Symbol, Sexpr, Qexpr, Expr, Lispy);

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
            
            lval* x = lval_eval(lval_read(r.output));
            lval_println(x);
            lval_del(x);
            
            mpc_ast_delete(r.output);
        } else {
            /* Parse unsuccessful */
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }
        
        
        free(input);
    }
    
    /* undefine and delete our parsers */
    mpc_cleanup(6, Number, Symbol, Sexpr, Qexpr, Expr, Lispy);
    
    return 0;
}

/* evaluates an S-expression */
lval* lval_eval_sexpr(lval* v) {
    /*Evaluate Children */
    for(int i = 0; i < v->count; i++) {
        v->cell[i] = lval_eval(v->cell[i]);
    }
        
    /* error checking */
    for(int i = 0; i < v->count; i++) {
        if(v->cell[i]->type == LVAL_ERR) {return lval_take(v, i);}
    }
    
    /* Empty Expression */
    if(v->count == 0) {return v;}
    
    /*Single expression */
    if(v->count == 1) {return lval_take(v, 0);}
    
    /* Ensure first element is a symbol */
    lval* f = lval_pop(v, 0);
    if(f->type != LVAL_SYM) {
        lval_del(f);
        lval_del(v);
        return lval_err("S-expression does not start with symbol!");
    }
    /* call builtin with operator */
    lval* result = builtin(v, f->sym);
    lval_del(f);
    return result;
}

lval* lval_eval(lval* v) {
    /* evaluate s-expressions */
    if(v->type == LVAL_SEXPR) {return lval_eval_sexpr(v);}
    /* all other lval types remain the same */
    return v;
}

/* allocates which builtin function to use */
lval* builtin(lval* a, char* func) {
    if(strcmp("list", func) == 0) {return builtin_list(a);}
    if (strcmp("head", func) == 0) { return builtin_head(a); }
    if (strcmp("tail", func) == 0) { return builtin_tail(a); }
    if (strcmp("join", func) == 0) { return builtin_join(a); }
    if (strcmp("eval", func) == 0) { return builtin_eval(a); }
    if (strstr("+-/*%^", func)) { return builtin_op(a, func); }
    lval_del(a);
    return lval_err("Unknown Function!");
}

lval* builtin_op(lval* a, char* op) {
  
    /* Ensure all arguments are numbers */
    for (int i = 0; i < a->count; i++) {
        if (a->cell[i]->type != LVAL_NUM) {
            lval_del(a);
            return lval_err("Cannot operate on non-number!");
        }
    }
  
    /* Pop the first element */
    lval* x = lval_pop(a, 0);

    /* If no arguments and sub then perform unary negation */
    if ((strcmp(op, "-") == 0) && a->count == 0) {
        x->num = -x->num;
    }

    /* While there are still elements remaining */
    while (a->count > 0) {

        /* Pop the next element */
        lval* y = lval_pop(a, 0);

        if (strcmp(op, "+") == 0) { x->num += y->num; }
        if (strcmp(op, "-") == 0) { x->num -= y->num; }
        if (strcmp(op, "*") == 0) { x->num *= y->num; }
        if (strcmp(op, "/") == 0) {
            if (y->num == 0) {
                lval_del(x); lval_del(y);
                x = lval_err("Division By Zero!"); break;
            }
            x->num /= y->num;
        }
        if(strcmp(op, "%") == 0) {
            if(y->num == 0) {
                lval_del(x); lval_del(y);
                x = lval_err("Division By Zero!"); break;
            }
            x->num %= y->num;
        }
        if(strcmp(op, "^") == 0) {
            x->num = (long) pow(x->num, y->num);
        }

        lval_del(y);
    }

    lval_del(a); return x;
}

/* returns the first element of a q-expression as a new q-expression */
/* frees the rest */
lval* builtin_head(lval* a) {
    LASSERT(a, a->count == 1,
        "Function 'head' passed too many arguments!");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
        "Function 'head' passed incorrect type!");
    LASSERT(a, a->cell[0]->count != 0,
        "Function 'head' passed {}!");
    lval* v = lval_take(a, 0);
    while(v->count > 1) {lval_del(lval_pop(v, 1));}
    return v;
}

/* returns a q-expression with the first element removed */
lval* builtin_tail(lval* a){
    LASSERT(a, a->count == 1,
        "Function 'tail' passed too many arguments!");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
        "Function 'tail' passed incorrect type!");
    LASSERT(a, a->cell[0]->count != 0,
        "Function 'tail' passed {}!");
        
    lval* v = lval_take(a, 0);
    lval_del(lval_pop(v, 0));
    return v;
}

/* converts an s-expression into a q-expression */
lval* builtin_list(lval* a) {
    a->type = LVAL_QEXPR;
    return a;
}

/* converts a q-expression into an s-expression and evaluates it */
lval* builtin_eval(lval* a) {
    LASSERT(a, a->count == 1,
        "Function 'eval' passed too many arguments!");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
        "Function 'eval' passed incorrect type!");
    
    lval* x = lval_take(a, 0);
    x->type = LVAL_SEXPR;
    return lval_eval(x);
}

/* joins multiple q-expressions together */
lval* builtin_join(lval* a) {
    for(int i = 0; i < a->count; i++) {
        LASSERT(a, a->cell[i]->type == LVAL_QEXPR,
            "Function 'join' passed incorrect type!");
    }
    
    lval* x = lval_pop(a, 0);
    
    while(a->count) {
        x = lval_join(x, lval_pop(a, 0));
    }
    
    lval_del(a);
    return x;
}


/* create a new number type lval */
lval* lval_num(long x) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_NUM;
    v->num = x;
    return v;
}

/* create a new error type lval */
lval* lval_err(char* m) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_ERR;
    v->err = malloc(strlen(m) + 1);
    strcpy(v->err, m);
    return v;
}

/* construct a pointer to a new symbol lval */
lval* lval_sym(char* s) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SYM;
    v->sym = malloc(strlen(s) + 1);
    strcpy(v->sym, s);
    return v;
}

/* construct a pointer to a new empty Sexpr lval */
lval* lval_sexpr(void) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

/* construct a pointer to a new empty Qexpr lval */
lval* lval_qexpr(void) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_QEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

/* construct a pointer to a new function */
lval* lval_fun(lbuiltin func) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_FUN;
    v->fun = func;
    return v;
}

lval* lval_read_num(mpc_ast_t* t) {
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ?
        lval_num(x) : lval_err("invalid number");
}

lval* lval_read(mpc_ast_t* t) {
    /* if symbol or number return conversion to that type */
    if (strstr(t->tag, "number")) {return lval_read_num(t);}
    if (strstr(t->tag, "symbol")) {return lval_sym(t->contents);}
    
    /* if root (>) or sexpr then create empty list */
    lval* x = NULL;
    if (strcmp(t->tag, ">") == 0) {x = lval_sexpr();}
    if(strstr(t->tag, "sexpr")) {x = lval_sexpr();}
    if (strstr(t->tag, "qexpr")) {x = lval_qexpr();}
    
    /*fill this list with valid expression contained within */
    for(int i = 0; i < t->children_num; i++) {
        if(strcmp(t->children[i]->contents, "(") == 0) { continue; }
        if(strcmp(t->children[i]->contents, ")") == 0) { continue; }
        if(strcmp(t->children[i]->contents, "{") == 0) { continue; }
        if(strcmp(t->children[i]->contents, "}") == 0) { continue; }
        if(strcmp(t->children[i]->tag, "regex") == 0) { continue; }
        x = lval_add(x, lval_read(t->children[i]));
    }
    return x;
}

lval* lval_add(lval* v, lval* x) {
    v->count++;
    v->cell = realloc(v->cell, sizeof(lval*) * v->count);
    v->cell[v->count-1] = x;
    return v;
}

/* removes and returns a child element of an s-expression */
lval* lval_pop(lval* v, int i) {
    /* find the item at "i" */
    lval* x = v->cell[i];
    
    /* shift memory after the item at "i" over the top */
    memmove(&v->cell[i], &v->cell[i+1],
        sizeof(lval*) * (v->count-i-1));
        
    /* decrease the count of items in the list */
    v->count--;
    
    /* reallocate the memory used */
    v->cell = realloc(v->cell, sizeof(lval*) * v->count);
    return x;
}

/* returns one element of an s-expression and frees the rest */
lval* lval_take(lval* v, int i) {
    lval* x = lval_pop(v, i);
    lval_del(v);
    return x;
}

/* joins one q-expression to another */
lval* lval_join(lval* x, lval* y) {
    /* for each cell in 'y' add it to 'x' */
    while(y->count) {
        x = lval_add(x, lval_pop(y, 0));
    }
    
    /* delete the empty 'y' and return 'x' */
    lval_del(y);
    return x;
}

/* creates a copy of an lval */
lval* lval_copy(lval* v) {
    lval* x = malloc(sizeof(lval));
    x->type = v->type;
    
    switch(v->type) {
        /* copy functions and numbers directly */
        case LVAL_FUN: x->fun = v->fun; break;
        case LVAL_NUM: x->num = v->num; break;
        
        /* copy strings using malloc and strcpy */
        case LVAL_ERR:
            x->err = malloc(strlen(v->err) + 1);
            strcpy(x->err, v->err); break;
            
        case LVAL_SYM:
            x->sym = malloc(strlen(v->sym) + 1);
            strcpy(x->sym, v->sym); break;
            
        /* copy lists by copying each sub-expression */
        case LVAL_SEXPR:
        case LVAL_QEXPR:
            x->count = v->count;
            x->cell = malloc(sizeof(lval*) * x->count);
            for(int i = 0; i < x->count; i++) {
                x->cell[i] = lval_copy(v->cell[i]);
            }
            break;
    }
    return x;
}

/* frees all allocated memory associated with an lval */
void lval_del(lval* v) {
    /* check which type it is */
    switch(v->type) {
        /*do nothing special for number type */
        case LVAL_NUM: break;
        
        /* for err or sym, free the string data */
        case LVAL_ERR: free(v->err); break;
        case LVAL_SYM: free(v->sym); break;
        case LVAL_FUN: break;
        
        /* if Sexpr/Qexpr then delete all elements inside */
        case LVAL_QEXPR:
        case LVAL_SEXPR:
            for(int i = 0; i< v->count; i++) {
                lval_del(v->cell[i]);
            }
            /* also free the memory allocated to contain the pointers */
            free(v->cell);
        break;
    }
    /* free memory allocated for "lval" itself */
    free(v);
}

/* print an "lval" */
void lval_print(lval* v) {
    switch (v->type) {
        /* if it is a number */
        case LVAL_NUM: 
            printf("%li", v->num); 
            break;
            
        /* if it is an error */
        case LVAL_ERR:
            /* print error */
            printf("Error: %s", v->err);
            break;
        case LVAL_SYM:
            printf("%s", v->sym);
            break;
        case LVAL_FUN:
            printf("<function>");
            break;
        case LVAL_SEXPR:
            lval_expr_print(v, '(', ')');
            break;
        case LVAL_QEXPR:
            lval_expr_print(v, '{', '}');
            break;
    }
}

/* print an "lval" followed by a newline */
void lval_println(lval* v) {
    lval_print(v);
    putchar('\n');
}

void lval_expr_print(lval* v, char open, char close) {
    putchar(open);
    for(int i = 0; i < v->count; i++) {
        /*print value contained within */
        lval_print(v->cell[i]);
        
        /* Don't print trailing space if last element */
        if(i != (v->count - 1)) {
            putchar(' ');
        }
    }
    putchar(close);
}

/* construct a new lenv */
lenv* lenv_new(void) {
    lenv* e = malloc(sizeof(lenv));
    e->count = 0;
    e->syms = NULL;
    e->vals = NULL;
    return e;
}

/* deletes an lenv */
void lenv_del(lenv* e) {
    for(int i = 0; i < e->count; i++) {
        free(e->syms[i]);
        lval_del(e->vals[i]);
    }
    free(e->syms);
    free(e->vals);
    free(e);
}





