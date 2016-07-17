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

#define LASSERT(args, cond, fmt, ...) \
    if(!(cond)) { \
        lval* err = lval_err(fmt, ##__VA_ARGS__); \
        lval_del(args); \
        return err; \
        }

#define LASSERT_TYPE(func, args, index, expect) \
  LASSERT(args, args->cell[index]->type == expect, \
    "Function '%s' passed incorrect type for argument %i. Got %s, Expected %s.", \
    func, index, ltype_name(args->cell[index]->type), ltype_name(expect))

#define LASSERT_NUM(func, args, num) \
  LASSERT(args, args->count == num, \
    "Function '%s' passed incorrect number of arguments. Got %i, Expected %i.", \
    func, args->count, num)

#define LASSERT_NOT_EMPTY(func, args, index) \
  LASSERT(args, args->cell[index]->count != 0, \
    "Function '%s' passed {} for argument %i.", func, index);
    
/* forward declare types */
struct lval;
struct lenv;
typedef struct lval lval;
typedef struct lenv lenv;

/* forward declare parsers */
mpc_parser_t* Number; 
mpc_parser_t* Symbol; 
mpc_parser_t* String; 
mpc_parser_t* Comment;
mpc_parser_t* Sexpr;  
mpc_parser_t* Qexpr;  
mpc_parser_t* Expr; 
mpc_parser_t* Lispy;

/* enum of possible lval types */
enum { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_STR,
    LVAL_SEXPR, LVAL_FUN, LVAL_QEXPR, LVAL_EXIT };
    
typedef lval*(*lbuiltin)(lenv*, lval*);

/* declare lval structure*/
struct lval{
    int type;
    
    long num;
    /* Error and Symbol types have some string data*/
    char* err;
    char* sym;
    char* str;
    
    /* Function */
    lbuiltin builtin;
    lenv* env;
    lval* formals;
    lval* body;
    
    /* Expression */
    int count;
    lval** cell;
};

/* declare lenv structure */
struct lenv {
    lenv* par;
    int count;
    char** syms;
    lval** vals;
};

/* declare eval methods - (bodies are directly after main) */
lval* lval_eval_sexpr(lenv* e, lval* v);
lval* lval_eval(lenv* e, lval* v);
lval* builtin(lval* a, char* func);
lval* builtin_def(lenv* e, lval* a);
lval* builtin_put(lenv* e, lval* a);
lval* builtin_var(lenv* e, lval* a, char* func);
lval* builtin_op(lenv* e, lval* a, char* op);
lval* builtin_ord(lenv* e, lval* a, char* op);
lval* builtin_cmp(lenv* e, lval* a, char* op);
lval* builtin_head(lenv* e, lval* a);
lval* builtin_tail(lenv* e, lval* a);
lval* builtin_list(lenv* e, lval* a);
lval* builtin_eval(lenv* e, lval* a);
lval* builtin_join(lenv* e, lval* a);
lval* builtin_add(lenv* e, lval* a);
lval* builtin_sub(lenv* e, lval* a);
lval* builtin_mul(lenv* e, lval* a);
lval* builtin_div(lenv* e, lval* a);
lval* builtin_lenv_print(lenv* e, lval* a);
lval* builtin_lambda(lenv* e, lval* a);
lval* builtin_gt(lenv* e, lval* a);
lval* builtin_lt(lenv* e, lval* a);
lval* builtin_ge(lenv* e, lval* a);
lval* builtin_le(lenv* e, lval* a);
lval* builtin_eq(lenv* e, lval* a);
lval* builtin_ne(lenv* e, lval* a);
lval* builtin_if(lenv* e, lval* a);
lval* builtin_load(lenv* e, lval* a);
lval* builtin_print(lenv* e, lval* a);
lval* builtin_error(lenv* e, lval* a);


/* declare lval methods */
lval* lval_num(long x);
lval* lval_err(char* fmt, ...);
lval* lval_sym(char* s);
lval* lval_sexpr(void);
lval* lval_qexpr(void);
lval* lval_fun(lbuiltin func);
lval* lval_lambda(lval* formals, lval* body);
lval* lval_str(char* s);
lval* lval_exit();
lval* lval_read_num(mpc_ast_t* t);
lval* lval_read_str(mpc_ast_t* t);
lval* lval_read(mpc_ast_t* t);
lval* lval_add(lval* v, lval* x);
lval* lval_pop(lval* v, int i);
lval* lval_take(lval* v, int i);
lval* lval_join(lval* x, lval* y);
lval* lval_copy(lval* v);
lval* lval_call(lenv* e, lval* f, lval* a);
int lval_eq(lval* x, lval* y);
void lval_del(lval* v);
void lval_print(lval* v);
void lval_println(lval* v);
void lval_expr_print(lval* v, char open, char close);
void lval_print_str(lval* v);

/*declare lenv methods */
void lenv_add_builtin(lenv* e, char* name, lbuiltin func);
void lenv_add_builtins(lenv* e);
lenv* lenv_new(void);
void lenv_del(lenv* e);
lval* lenv_get(lenv* e, lval* k);
void lenv_put(lenv* e, lval* k, lval* v);
void lenv_def(lenv* e, lval* k, lval* v);
lenv* lenv_copy(lenv* e);

/* other methods */
char* ltype_name(int t);



int main(int argc, char** argv) {
    
    /* set up our grammer */
    /*create our parsers */
    Number = mpc_new("number");
    Symbol = mpc_new("symbol");
    String = mpc_new("string");
    Comment = mpc_new("comment");
    Sexpr = mpc_new("sexpr");
    Qexpr = mpc_new("qexpr");
    Expr = mpc_new("expr");
    Lispy = mpc_new("lispy");
    
    /* define the language */
    mpca_lang(MPCA_LANG_DEFAULT,
        "                                                                           \
            number      : /-?[0-9]+/ ;                                              \
            symbol      : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;                        \
            string      : /\"(\\\\.|[^\"])*\"/ ;                                    \
            comment     : /;[^\\r\\n]*/ ;                                           \
            sexpr       : '(' <expr>* ')' ;                                         \
            qexpr       : '{' <expr>* '}' ;                                         \
            expr        : <number> | <string> | <symbol>                            \
                        | <comment> | <sexpr> | <qexpr> ;                           \
            lispy       : /^/ <expr>* /$/ ;                                         \
        ",
        Number, Symbol, String, Comment, Sexpr, Qexpr, Expr, Lispy);

    /* Print version and exit info */
    puts("Lisperer Version 0.0.0.1");
    puts("exit() to quit \n");
    
    lenv* e = lenv_new();
    lenv_add_builtins(e);
    
    /* load standard library */
    lval* libr = lval_add(lval_sexpr(), lval_str("stlib.lspy"));
    lval* res = builtin_load(e, libr);
    if (res->type == LVAL_ERR) { lval_println(res); }
    lval_del(res);
    lval_del(libr);
    
    if(argc == 1) {
    
        /* In a never ending loop */
        while(1) {
            
            
            /* now the readline function will work on both operating systems*/
            char* input = readline("lisperer> ");
            add_history(input);
            
            /* parse input */
            mpc_result_t r;
            if (mpc_parse("<stdin>", input, Lispy, &r)) {
                /*parse successful */
                lval* x = lval_eval(e, lval_read(r.output));
                
                lval_println(x);
                
                if(x->type == LVAL_EXIT){
                    lval_del(x);
                    mpc_ast_delete(r.output);
                    free(input);
                    break;
                }
                lval_del(x);
                
                mpc_ast_delete(r.output);
                
                
            } else {
                /* Parse unsuccessful */
                mpc_err_print(r.error);
                mpc_err_delete(r.error);
            }
            
            
            free(input);
            
        }
    }
    
    /* if supplied with a list of files */
    if (argc >= 2) {
  
        /* loop over each supplied filename (starting from 1) */
        for (int i = 1; i < argc; i++) {
          
            /* Argument list with a single argument, the filename */
            lval* args = lval_add(lval_sexpr(), lval_str(argv[i]));
          
            /* Pass to builtin load and get the result */
            lval* x = builtin_load(e, args);
          
            /* If the result is an error be sure to print it */
            if (x->type == LVAL_ERR) { lval_println(x); }
            lval_del(x);
        }
    }
    
    lenv_del(e);
    
    /* undefine and delete our parsers */
    mpc_cleanup(8, Number, Symbol, String, Comment, Sexpr, Qexpr, Expr, Lispy);
    
    return 0;
}

/* evaluates an S-expression */
lval* lval_eval_sexpr(lenv* e, lval* v) {
    /*Evaluate Children */
    for(int i = 0; i < v->count; i++) {
        v->cell[i] = lval_eval(e, v->cell[i]);
    }
        
    /* error checking */
    for(int i = 0; i < v->count; i++) {
        if(v->cell[i]->type == LVAL_ERR) {return lval_take(v, i);}
    }
    
    /* Empty Expression */
    if(v->count == 0) {return v;}
    
    /*Single expression */
    if(v->count == 1) {return lval_take(v, 0);}
    
    /* Ensure first element is a function after evaluation */
    lval* f = lval_pop(v, 0);
    if(f->type == LVAL_EXIT) {
        lval_del(f);
        lval_del(v);
        return lval_exit();
    }
    if(f->type != LVAL_FUN) {
        lval* err = lval_err(
            "S-Expression starts with incorrect type. "
            "Got %s, Expected %s.",
            ltype_name(f->type), ltype_name(LVAL_FUN));
        lval_del(f); 
        lval_del(v);
        return err;
    }
    /* call the function to get the result */
    lval* result = lval_call(e, f, v);
    lval_del(f);
    return result;
}

lval* lval_eval(lenv* e, lval* v) {
    if(v->type == LVAL_SYM) {
        lval* x = lenv_get(e, v);
        lval_del(v);
        return x;
    }
    
    /* evaluate s-expressions */
    if(v->type == LVAL_SEXPR) {return lval_eval_sexpr(e, v);}
    /* all other lval types remain the same */
    return v;
}

lval* builtin_def(lenv* e, lval* a) {
    return builtin_var(e, a, "def");
}

lval* builtin_put(lenv* e, lval* a) {
    return builtin_var(e, a, "=");
}

lval* builtin_var(lenv* e, lval* a, char* func) {
    LASSERT_TYPE(func, a, 0, LVAL_QEXPR);
        
    /*first argument is symbol list */
    lval* syms = a->cell[0];
        
    /* ensure all elements of first list are symbols */
    for(int i = 0; i < syms->count; i++){
        LASSERT(a, syms->cell[i]->type == LVAL_SYM,
            "Function '%s' cannot define non-symbol. "
            "Got %s, Expected %s.", func,
            ltype_name(syms->cell[i]->type), ltype_name(LVAL_SYM));
    }
    /* check correct number of symbols and values */
    LASSERT(a, (syms->count == a->count-1),
        "Function '%s' passed too many arguments for symbols. "
        "Got %i, Expected %i.", func, syms->count, a->count-1);
        
    /* assign copies of values to symbols */
    for (int i = 0; i < syms->count; i++) {
        /* If 'def' define in globally. If 'put' define in locally */
        if (strcmp(func, "def") == 0) {
            lenv_def(e, syms->cell[i], a->cell[i+1]);
        }
    
        if (strcmp(func, "=")   == 0) {
            lenv_put(e, syms->cell[i], a->cell[i+1]);
        } 
    }
  
    lval_del(a);
    return lval_sexpr();
}


lval* builtin_op(lenv* e, lval* a, char* op) {
  
    /* Ensure all arguments are numbers */
    for (int i = 0; i < a->count; i++) {
        LASSERT_TYPE(op, a, i, LVAL_NUM);
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

/* compares the order of numeric values (<, >, <=, >=) */
lval* builtin_ord(lenv* e, lval* a, char* op) {
    LASSERT_NUM(op, a, 2);
    LASSERT_TYPE(op, a, 0, LVAL_NUM);
    LASSERT_TYPE(op, a, 1, LVAL_NUM);
    
    int r;
    if (strcmp(op, ">") == 0) {
        r = (a->cell[0]->num > a->cell[1]->num);
    }
    if (strcmp(op, "<")  == 0) {
        r = (a->cell[0]->num <  a->cell[1]->num);
    }
    if (strcmp(op, ">=") == 0) {
        r = (a->cell[0]->num >= a->cell[1]->num);
    }
    if (strcmp(op, "<=") == 0) {
        r = (a->cell[0]->num <= a->cell[1]->num);
    }
    lval_del(a);
    return lval_num(r);
}

lval* builtin_cmp(lenv* e, lval* a, char* op) {
    LASSERT_NUM(op, a, 2);
    int r;
    if(strcmp(op, "==") == 0) {
        r = lval_eq(a->cell[0], a->cell[1]);
    }
    if(strcmp(op, "!=") == 0) {
        r = !lval_eq(a->cell[0], a->cell[1]);
    }
    lval_del(a);
    return lval_num(r);
}

/* returns the first element of a q-expression as a new q-expression */
/* frees the rest */
lval* builtin_head(lenv* e, lval* a) {
    LASSERT_NUM("head", a, 1);
    LASSERT_TYPE("head", a, 0, LVAL_QEXPR);
    LASSERT_NOT_EMPTY("head", a, 0);
    lval* v = lval_take(a, 0);
    while(v->count > 1) {lval_del(lval_pop(v, 1));}
    return v;
}

/* returns a q-expression with the first element removed */
lval* builtin_tail(lenv* e, lval* a){
    LASSERT_NUM("tail", a, 1);
    LASSERT_TYPE("tail", a, 0, LVAL_QEXPR);
    LASSERT_NOT_EMPTY("tail", a, 0);
        
    lval* v = lval_take(a, 0);
    lval_del(lval_pop(v, 0));
    return v;
}

/* converts an s-expression into a q-expression */
lval* builtin_list(lenv* e, lval* a) {
    a->type = LVAL_QEXPR;
    return a;
}

/* converts a q-expression into an s-expression and evaluates it */
lval* builtin_eval(lenv* e, lval* a) {
    LASSERT_NUM("eval", a, 1);
    LASSERT_TYPE("eval", a, 0, LVAL_QEXPR);
    
    lval* x = lval_take(a, 0);
    x->type = LVAL_SEXPR;
    return lval_eval(e, x);
}

/* joins multiple q-expressions together */
lval* builtin_join(lenv* e, lval* a) {
    for(int i = 0; i < a->count; i++) {
        LASSERT_TYPE("join", a, i, LVAL_QEXPR);
    }
    
    lval* x = lval_pop(a, 0);
    
    while(a->count) {
        x = lval_join(x, lval_pop(a, 0));
    }
    
    lval_del(a);
    return x;
}

/* function for adding */
lval* builtin_add(lenv* e, lval* a) {
    return builtin_op(e, a, "+");
}

/* function for subtracting */
lval* builtin_sub(lenv* e, lval* a) {
    return builtin_op(e, a, "-");
}

/* function for multiplying */
lval* builtin_mul(lenv* e, lval* a) {
    return builtin_op(e, a, "*");
}

lval* builtin_div(lenv* e, lval* a) {
    return builtin_op(e, a, "/");
}

lval* builtin_exit(lenv* e, lval* a) {
    return lval_exit();
}

lval* builtin_lenv_print(lenv* e, lval* a) {
    for(int i = 0; i < e->count; i++) {
        printf("%s: ", e->syms[i]);
        lval_println(e->vals[i]);
        
    }
    lval_del(a);
    return lval_sexpr();
}

lval* builtin_lambda(lenv* e, lval* a) {
    /* Check Two arguments, each of which are Q-Expressions */
    LASSERT_NUM("\\", a, 2);
    LASSERT_TYPE("\\", a, 0, LVAL_QEXPR);
    LASSERT_TYPE("\\", a, 1, LVAL_QEXPR);
    
    /* Check first Q-Expression contains only Symbols */
    for (int i = 0; i < a->cell[0]->count; i++) {
        LASSERT(a, (a->cell[0]->cell[i]->type == LVAL_SYM),
            "Cannot define non-symbol. Got %s, Expected %s.",
            ltype_name(a->cell[0]->cell[i]->type),ltype_name(LVAL_SYM));
    }
    
    /* Pop first two arguments and pass them to lval_lambda */
    lval* formals = lval_pop(a, 0);
    lval* body = lval_pop(a, 0);
    lval_del(a);
    
    return lval_lambda(formals, body);
    
}

lval* builtin_gt(lenv* e, lval* a) {
    return builtin_ord(e, a, ">");
}

lval* builtin_lt(lenv* e, lval* a) {
    return builtin_ord(e, a, "<");
}

lval* builtin_ge(lenv* e, lval* a) {
    return builtin_ord(e, a, ">=");
}

lval* builtin_le(lenv* e, lval* a) {
    return builtin_ord(e, a, "<=");
}

lval* builtin_eq(lenv* e, lval* a) {
    return builtin_cmp(e, a, "==");
}

lval* builtin_ne(lenv* e, lval* a) {
    return builtin_cmp(e, a, "!=");
}

/* function for if statements */
lval* builtin_if(lenv* e, lval* a) {
    LASSERT_NUM("if", a, 3);
    LASSERT_TYPE("if", a, 0, LVAL_NUM);
    LASSERT_TYPE("if", a, 1, LVAL_QEXPR);
    LASSERT_TYPE("if", a, 2, LVAL_QEXPR);
    
    /* Mark both expressions as evaluable (s-expressions) */
    lval* x;
    a->cell[1]->type = LVAL_SEXPR;
    a->cell[2]->type = LVAL_SEXPR;
    
    if(a->cell[0]->num) {
        /* if condition is true, evaluate first expression */
        x = lval_eval(e, lval_pop(a, 1));
    } else {
        /* if false, evaluate second expression */
        x = lval_eval(e, lval_pop(a, 2));
    }
    
    /* Delete argument list and return */
    lval_del(a);
    return x;
}

/* loads and evaluates an external file */
lval* builtin_load(lenv* e, lval* a){
    LASSERT_NUM("load", a, 1);
    LASSERT_TYPE("load", a, 0, LVAL_STR);
    
    /* parse file given by string name */
    mpc_result_t r;
    if(mpc_parse_contents(a->cell[0]->str, Lispy, &r)) {
        /* read contents */
        lval* expr = lval_read(r.output);
        mpc_ast_delete(r.output);
        
        /* evaluate each expression */
        while(expr->count) {
            lval* x = lval_eval(e, lval_pop(expr, 0));
            /* if evaluation leads to error print it */
            if(x->type == LVAL_ERR) {
                lval_println(x);
            }
            lval_del(x);
        }
        /* delete expressions and arguments */
        lval_del(expr);
        lval_del(a);
        /* return empty list */
        return lval_sexpr();
    } else {
        /* get parse error as string */
        char* err_msg = mpc_err_string(r.error);
        mpc_err_delete(r.error);
        
        /* create new error message using it */
        lval* err = lval_err("Could not load Library %s", err_msg);
        
        /* clean up and return error */
        free(err_msg);
        lval_del(a);
        return err;
    }
}

lval* builtin_print(lenv* e, lval* a) {

    /* Print each argument followed by a space */
    for (int i = 0; i < a->count; i++) {
        lval_print(a->cell[i]); putchar(' ');
    }

    /* Print a newline and delete arguments */
    putchar('\n');
    lval_del(a);

    return lval_sexpr();
}

lval* builtin_error(lenv* e, lval* a) {
    LASSERT_NUM("error", a, 1);
    LASSERT_TYPE("error", a, 0, LVAL_STR);

    /* Construct Error from first argument */
    lval* err = lval_err(a->cell[0]->str);

    /* Delete arguments and return */
    lval_del(a);
    return err;
}

/* create a new number type lval */
lval* lval_num(long x) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_NUM;
    v->num = x;
    return v;
}

/* create a new error type lval */
lval* lval_err(char* fmt, ...) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_ERR;
    
    /* create a va list and initialize it */
    va_list va;
    va_start(va, fmt);
    
    /* allocate 512 bytes */
    v->err = malloc(512);
    /*print the error string with max of 511 characters */
    vsnprintf(v->err, 511, fmt, va);
    
    /* reallocate to number of bytes actually used */
    v->err = realloc(v->err, strlen(v->err)+1);
    
    /* clean up our va list */
    va_end(va);
    
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
    v->builtin = func;
    return v;
}

/* constructor for user defined functions */
lval* lval_lambda(lval* formals, lval* body) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_FUN;
    
    /* set builtin to null */
    v->builtin = NULL;
    
    /* build new environment */
    v->env = lenv_new();
    
    /* set formals and body */
    v->formals = formals;
    v->body = body;
    return v;
}

/* constructor for string lvals */
lval* lval_str(char* s) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_STR;
    v->str = malloc(strlen(s) + 1);
    strcpy(v->str, s);
    return v;
}

lval* lval_exit() {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_EXIT;
    return v;
}

lval* lval_read_num(mpc_ast_t* t) {
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ?
        lval_num(x) : lval_err("invalid number");
}

lval* lval_read_str(mpc_ast_t* t) {
    /* cut off the final quote character */
    t->contents[strlen(t->contents)-1] = '\0';
    /* copy the string missing out the first quote character */
    char* unescaped = malloc(strlen(t->contents+1)+1);
    strcpy(unescaped, t->contents+1);
    /* pass through the unescape function */
    unescaped = mpcf_unescape(unescaped);
    /*construct a new lval using the string */
    lval* str = lval_str(unescaped);
    /* free the string and return */
    free(unescaped);
    return str;
}

lval* lval_read(mpc_ast_t* t) {
    /* if symbol or number return conversion to that type */
    if (strstr(t->tag, "number")) {return lval_read_num(t);}
    if (strstr(t->tag, "symbol")) {return lval_sym(t->contents);}
    if (strstr(t->tag, "string")) {return lval_read_str(t);}
    
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
        if(strstr(t->children[i]->tag, "comment")) {continue;}
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
        case LVAL_FUN: 
            if(v->builtin) {
                x->builtin = v->builtin;
            } else {
                x->builtin = NULL;
                x->env = lenv_copy(v->env);
                x->formals = lval_copy(v->formals);
                x->body = lval_copy(v->body);
            }
 
            break;
        case LVAL_NUM: x->num = v->num; break;
        
        /* copy strings using malloc and strcpy */
        case LVAL_ERR:
            x->err = malloc(strlen(v->err) + 1);
            strcpy(x->err, v->err); break;
            
        case LVAL_SYM:
            x->sym = malloc(strlen(v->sym) + 1);
            strcpy(x->sym, v->sym); break;
            
        case LVAL_STR:
            x->str = malloc(strlen(v->str) + 1);
            strcpy(x->str, v->str);
            break;
            
        /* copy lists by copying each sub-expression */
        case LVAL_SEXPR:
        case LVAL_QEXPR:
            x->count = v->count;
            x->cell = malloc(sizeof(lval*) * x->count);
            for(int i = 0; i < x->count; i++) {
                x->cell[i] = lval_copy(v->cell[i]);
            }
            break;
        case LVAL_EXIT: break;
    }
    return x;
}


/* calls a function */
lval* lval_call(lenv* e, lval* f, lval* a) {

    /* If Builtin then simply apply that */
    if (f->builtin) { return f->builtin(e, a); }

    /* Record Argument Counts */
    int given = a->count;
    int total = f->formals->count;

    /* While arguments still remain to be processed */
    while (a->count) {

        /* If we've ran out of formal arguments to bind */
        if (f->formals->count == 0) {
            lval_del(a); return lval_err(
                "Function passed too many arguments. "
                "Got %i, Expected %i.", given, total); 
        }

        /* Pop the first symbol from the formals */
        lval* sym = lval_pop(f->formals, 0);
        
        /* Special Case to deal with '&' */
        if (strcmp(sym->sym, "&") == 0) {
          
            /* Ensure '&' is followed by another symbol */
            if (f->formals->count != 1) {
                lval_del(a);
                return lval_err("Function format invalid. "
                    "Symbol '&' not followed by single symbol.");
            }
          
            /* Next formal should be bound to remaining arguments */
            lval* nsym = lval_pop(f->formals, 0);
            lenv_put(f->env, nsym, builtin_list(e, a));
            lval_del(sym); lval_del(nsym);
            break;
        }

        /* Pop the next argument from the list */
        lval* val = lval_pop(a, 0);

        /* Bind a copy into the function's environment */
        lenv_put(f->env, sym, val);

        /* Delete symbol and value */
        lval_del(sym); lval_del(val);
    }

    /* Argument list is now bound so can be cleaned up */
    lval_del(a);
    
    /* If '&' remains in formal list bind to empty list */
    if (f->formals->count > 0 &&
        strcmp(f->formals->cell[0]->sym, "&") == 0) {
        
        /* Check to ensure that & is not passed invalidly. */
        if (f->formals->count != 2) {
            return lval_err("Function format invalid. "
                "Symbol '&' not followed by single symbol.");
        }
        
        /* Pop and delete '&' symbol */
        lval_del(lval_pop(f->formals, 0));
        
        /* Pop next symbol and create empty list */
        lval* sym = lval_pop(f->formals, 0);
        lval* val = lval_qexpr();
        
        /* Bind to environment and delete */
        lenv_put(f->env, sym, val);
        lval_del(sym); lval_del(val);
    }

    /* If all formals have been bound evaluate */
    if (f->formals->count == 0) {

        /* Set environment parent to evaluation environment */
        f->env->par = e;

        /* Evaluate and return */
        return builtin_eval(
            f->env, lval_add(lval_sexpr(), lval_copy(f->body)));
    } else {
        /* Otherwise return partially evaluated function */
        return lval_copy(f);
    }

}

/* checks to see if two lvals are equal */
int lval_eq(lval* x, lval* y) {
    /* Different types are always unequal */
    if(x->type != y->type) {return 0;}
    
    /* Compare based upon type */
    switch(x->type) {
        case LVAL_NUM: return (x->num == y->num);
        
        case LVAL_ERR: return(strcmp(x->err, y->err) == 0);
        case LVAL_SYM: return(strcmp(x->sym, y->sym) == 0);
        case LVAL_STR: return(strcmp(x->str, y->str) == 0);
        
        /* compare if builtin, otherwise compare formals and body */
        case LVAL_FUN:
            if(x->builtin || y->builtin) {
                return x->builtin == y->builtin;
            } else {
                return lval_eq(x->formals, y->formals)
                    && lval_eq(x->body, y->body);
            }
        
        case LVAL_QEXPR:
        case LVAL_SEXPR:
            if(x->count != y->count) {return 0;}
            for(int i = 0; i < x->count; i++) {
                if(!lval_eq(x->cell[i], y->cell[i])) {return 0;}
            }
            /* otherwise lists must be equal */
            return 1;
            break;
    }
    return 0;
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
        case LVAL_STR: free(v->str); break;
        case LVAL_FUN: 
            if(!v->builtin){
                lenv_del(v->env);
                lval_del(v->formals);
                lval_del(v->body);
            }
            break;
        
        /* if Sexpr/Qexpr then delete all elements inside */
        case LVAL_QEXPR:
        case LVAL_SEXPR:
            for(int i = 0; i< v->count; i++) {
                lval_del(v->cell[i]);
            }
            /* also free the memory allocated to contain the pointers */
            free(v->cell);
        break;
        case LVAL_EXIT: break;
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
        case LVAL_STR:
            lval_print_str(v);
            break;
        case LVAL_FUN:
            if (v->builtin) {
                printf("<builtin>");
            } else {
                printf("(\\ "); lval_print(v->formals);
                putchar(' '); lval_print(v->body); putchar(')');
            }
            break;
        case LVAL_SEXPR:
            lval_expr_print(v, '(', ')');
            break;
        case LVAL_QEXPR:
            lval_expr_print(v, '{', '}');
            break;
        case LVAL_EXIT:
            printf("Exit call... \n");
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

/* print an lval string */
void lval_print_str(lval* v) {
    /* make a copy of the string */
    char* escaped = malloc(strlen(v->str)+1);
    strcpy(escaped, v->str);
    /* pass it through the escape function */
    escaped = mpcf_escape(escaped);
    /*print it between " characters */
    printf("\"%s\"", escaped);
    free(escaped);
}

/* add a builtin function to the environment */
void lenv_add_builtin(lenv* e, char* name, lbuiltin func) {
    lval* k = lval_sym(name);
    lval* v = lval_fun(func);
    lenv_put(e, k, v);
    lval_del(k);
    lval_del(v);
}

/* add all the starting function to the environment */
void lenv_add_builtins(lenv* e) {
    /* list functions */
    lenv_add_builtin(e, "list", builtin_list);
    lenv_add_builtin(e, "head", builtin_head);
    lenv_add_builtin(e, "tail", builtin_tail);
    lenv_add_builtin(e, "eval", builtin_eval);
    lenv_add_builtin(e, "join", builtin_join);
    
    /* mathematical functions */
    lenv_add_builtin(e, "+", builtin_add);
    lenv_add_builtin(e, "-", builtin_sub);
    lenv_add_builtin(e, "*", builtin_mul);
    lenv_add_builtin(e, "/", builtin_div);
    
    /* Variable functions */
    lenv_add_builtin(e, "\\",  builtin_lambda);
    lenv_add_builtin(e, "def", builtin_def);
    lenv_add_builtin(e, "=",   builtin_put);
    
    /* Comparision functions */
    lenv_add_builtin(e, "if", builtin_if);
    lenv_add_builtin(e, "==", builtin_eq);
    lenv_add_builtin(e, "!=", builtin_ne);
    lenv_add_builtin(e, ">",  builtin_gt);
    lenv_add_builtin(e, "<",  builtin_lt);
    lenv_add_builtin(e, ">=", builtin_ge);
    lenv_add_builtin(e, "<=", builtin_le);
    
    /* other */
    lenv_add_builtin(e, "exit", builtin_exit);
    lenv_add_builtin(e, "print_env", builtin_lenv_print);
    lenv_add_builtin(e, "load", builtin_load);
    lenv_add_builtin(e, "print", builtin_print);
    lenv_add_builtin(e, "error", builtin_error);
}

/* construct a new lenv */
lenv* lenv_new(void) {
    lenv* e = malloc(sizeof(lenv));
    e->par = NULL;
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

/* get a value from an lenv variable */
lval* lenv_get(lenv* e, lval* k) {
    /*iterate over all items in environment */
    for(int i = 0; i < e->count; i++) {
        /* Check if the stored string matches the symbol string */
        /* If it does, return a copy of the value */
        if(strcmp(e->syms[i], k->sym) == 0) {
            return lval_copy(e->vals[i]);
        }
    }
    
    /* if no symbol check in parent otherwise error */
    if(e->par) {
        return lenv_get(e->par, k);
    } else { 
        return lval_err("Unbound Symbol '%s'", k->sym);
    }
}

/* set a value for an lenv variable */
void lenv_put(lenv* e, lval* k, lval* v) {
    /* Iterate over all items in environment */
    /* This is to see if variable already exists */
    for(int i = 0; i < e->count; i++) {
        /* If variable is found delete item at that position */
        /* And replace with variable supplied by user */
        if(strcmp(e->syms[i], k->sym) == 0) {
            lval_del(e->vals[i]);
            e->vals[i] = lval_copy(v);
            return;
        }
    }
    
    /* if no existing entry found allocate space for new entry */
    e->count++;
    e->vals = realloc(e->vals, sizeof(lval*) * e->count);
    e->syms = realloc(e->syms, sizeof(char*) * e->count);
    
    /* copy contents of lval and symbol string into new location */
    e->vals[e->count-1] = lval_copy(v);
    e->syms[e->count-1] = malloc(strlen(k->sym+1));
    strcpy(e->syms[e->count-1], k->sym);
}

/* insert values into global environment */
void lenv_def(lenv* e, lval* k, lval* v) {
    /* Iterate till e has no parent */
    while (e->par) { e = e->par; }
    /* Put value in e */
    lenv_put(e, k, v);
}

/* method for copying an environment */
lenv* lenv_copy(lenv* e) {
    lenv* n = malloc(sizeof(lenv));
    n->par = e->par;
    n->count = e->count;
    n->syms = malloc(sizeof(char*) * n->count);
    n->vals = malloc(sizeof(lval*) * n->count);
    for (int i = 0; i < e->count; i++) {
        n->syms[i] = malloc(strlen(e->syms[i]) + 1);
        strcpy(n->syms[i], e->syms[i]);
        n->vals[i] = lval_copy(e->vals[i]);
    }
    return n;
}



char* ltype_name(int t) {
    switch(t) {
        case LVAL_FUN: return "Function";
        case LVAL_NUM: return "Number";
        case LVAL_ERR: return "Error";
        case LVAL_SYM: return "Symbol";
        case LVAL_STR: return "String";
        case LVAL_SEXPR: return "S-expression";
        case LVAL_QEXPR: return "Q-expression";
        default: return "Unknown";
    }
}


