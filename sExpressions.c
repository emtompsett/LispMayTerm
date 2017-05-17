#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpc.h"
#include <editline/readline.h>

typedef struct //lisp value type, handles errors
{
	int type;
	double num;
	
	//Error and Symbol types have some string data
	char* err;
	char* sym;
	
	//Count and Pointer to a list of lval
	int count;
	struct lval** cell;
} lval;

//Enums for lval types, error type, symbols, and s-expressions
enum{ LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR};


//Constructors for pointers to lval
lval* lval_num(double x)
{
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_NUM;
	v->num = x;
	return v;
}

lval* lval_err(char* m)
{
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_ERR;
	v->err = malloc(strlen(s)+1);
	strcpy(v->sym, s);
	return v;
}

lval* lval_sexpr(void)
{
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_SEXPR;
	v->count = 0;
	v->cell = NULL;
	return v;
}

//Destructor for lval
void lval_del(lval* v)
{
	switch (v->type)
	{
		//Does nothing special for nums
		case LVAL_NUM: break; 

		//For Err or Sym free the string data
		case LVAL_ERR: free(v->err); break;
		case LVAL_SYM: free(v->err); break;

		//For s-expressions, free all components 
		case LVAL_SEXPR:
			for (int i = 0; i < v->count; i++)
				{
					lval_del(v->cell[i]);
				}
			free(v->cell); //memory allocated to pointers 
		break;
	}
	free(v) //actually freeing v
}

lval* lval_read_num(mpc_ast_t* t)
{
	errno = 0;
	double x = atof(t->contents);
	return errno != ERANGE ? lval_num(x) : lval_err("invalid number")
}

lval* lval_read(mpc_ast_t* t)
{
	//If Symbol or Number, return a conversion to that type
	if (strstr(t->tag, "number")) {return lval_read_num(t);}
	if (strstr(t->tag, "symbol")) {return lval_sym(t->contents);}

	//If root (>) or s-expression then create empty list
	lval* x = NULL;
	if (strcmp(t->tag, ">") == 0) {x = lval_sexpr();}
	if(strstr(t->tag, "sexpr")) {x = lval_sexpr();}

	//Fill this list with any valid expression contained within
	for (int i = 0, i < t->children_num; i++)
	{
		if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
		if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
		if (strcmp(t->children[i]->contents, "regex") == 0) { continue; }
		x = lval_add(x, lval_read(t->children[i]));
	}
	return x;
}

void lval_expr_print(lval* v); //forward declaration

//Lval print function without newline
void lval_print(lval* v) 
{
	switch (v->type)
	{
		//prints value depending on typer
		case LVAL_NUM: printf("%f", v->num); break; 
		case LVAL_ERR: printf("Error: %s", v->err); break;
		case LVAL_SYM: printf("%s", v->sym); break;
		case LVAL_SEXPR: lval_expr_print(v, '(', ')') break;
	}
}
//Lval print function with a newline 
void lval_println(lval* v) { lval_print(v); putchar('\n'); }

void lval_expr_print(lval* v, char open, char close)
{
	putchar(open);
	for (int i = 0; i < v-> count; i++)
	{
		lval_print(v->cell[i]); //prints contained lval

		//Doesn't print trailing space for last element
		if (i != (v->count -1)) { putchar(' '); }
	}
	putchar(close);
}

//Pops the element at i from the list
lval* lval_pop(lval* v, int i); 
{
	//Find the item at i
	lval* x = v->cell[i];

	//Shift memory after the item at i over the top
	memomve(&v->cell[i], &v->cell[i+1],
		sizeof(lval*) * (v->count-i-1));

	v->count--; //list got smaller

	//Reallocate the memory used 
	v->cell = realloc(v->cell, sizeof(lval*) * v->count)
	return x;
}

//special case of lval_pop that frees whole list after pop
lval* lval_take(lval* v, int i) 
{
	lval* x = lval_pop(v, i)l;
	lval_del(v);
	return x;
}

//evaluates all builtin operations
lval* builtin_op(lval* a, char* op)
{
//check all arguments are numbers 
	for (int i = 0; i < a->count; i++)
	{
		if (a->cell[i]->type != LVAL_NUM)
		{
			lval_del(a);
			return lval_err("Cannot operate on non-number");
		}
	}
	
	lval* x = lval_pop(a, 0); //pops the first element

	if ((strcmp(op, "-") == 0) && a->count == 0) //unary negation
	{
		x->num = -x->num;
	}

	//While there are still elements remaining
	while (a->count > 0)
	{
		lval* y = lval_pop(a, 0); //pop the next element

		if (strcmp(op, "add") == 0) { x->num += y->num; }
		if (strcmp(op, "sub") == 0) { x->num -= y->num; }
		if (strcmp(op, "mul") == 0) { x->num *= y->num; }
		if (strcmp(op, "exp") == 0) { x->num = powf(x->num,y->num); }
		if (strcmp(op, "div") == 0)
		{
			if (y->num == 0)
			{
				lval_del(x); lval_del(y);
				x = lval_err("Division by Zero!"); break;
			}
			x->num /= y->num;
		}

		lval_del(y);
	}

	lval_del(a); return x;
}

lval* lval_eval_sexpr(lval* v);
lval* lval_eval(lval* v);


int main(int argc, char** argv)
{
	//Creating parsers 
	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Symbol = mpc_new("symbol");
	mpc_parser_t* Sexpr = mpc_new("sexpr")
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Lispy = mpc_new("lispy");

	//Defining parsers with language
	mpca_lang(MPCA_LANG_DEFAULT,
		"\
			number : /-?[0-9]+\.?[0-9]*/; \
			symbol : /add/ | /sub/ | /mul/ | /div/ | /mod/ | /exp/; \
			sexpr : '(' <expr>* ')'; \
			expr : <number> | <symbol> | <sexpr> ; \
			lispy : /^/ <expr>* /$/; \
		",
		Number, Symbol, Sexpr, Expr, Lispy);

	puts("Lispy Version 0.0.0.4");
	puts("Now with error handling");
	puts("All numbers must be in decimal form\n");

	while (1)
	{
		//Output prompt to the user and get input and add it to history
		char* input = readline("lispy> ");
		add_history(input);
		mpc_result_t r;
		if (mpc_parse("<stdin>", input, Lispy, &r))
		{
			//Read, Eval, Print
			lval* x = lval_eval(lval_read(r.output));
			lval_println(x);
			lval_del(x);
		}

		else 
		{
			//Otherwise print the error
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}
		
		free(input);
	}
	
	mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lispy);
	return 0;
}