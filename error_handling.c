#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpc.h"
#include <editline/readline.h>

typedef struct //lisp value type, handles errors
{
	int type;
	float num;
	int err;
} lval;

//Enums for lval types and error type
enum{ LVAL_NUM, LVAL_ERR };
enum{ LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

//Constructors for lval
lval lval_num(float x)
{
	lval v;
	v.type = LVAL_NUM;
	v.num = x;
	return v;
}

lval lval_err(int x)
{
	lval v;
	v.type = LVAL_ERR;
	v.err = x;
	return v;
}

//Lval print function without newline
void lval_print(lval v) 
{
	switch (v.type)
	{
		case LVAL_NUM: printf("%f", v.num); break; //prints value if num

		case LVAL_ERR:
			if (v.err == LERR_DIV_ZERO)
			{
				printf("Error: Division By Zero!");
			}
			if (v.err == LERR_BAD_OP)
			{
				printf("Error: Invalid Operator");
			}
			if (v.err == LERR_BAD_NUM)
			{
				printf("Error: Invalid Number!");
			}
		break;
	}
}
//Lval print function with a newline 
void lval_println(lval v) { lval_print(v); putchar('\n'); }


lval eval_op(lval x, char* op, lval y) //numeric operation evaluation
{
	//If either value is an error, return them
	if (x.type == LVAL_ERR) {return x;}
	if (y.type == LVAL_ERR) {return y;}


	if (strcmp(op, "add") == 0) {return lval_num(x.num + y.num);}
	if (strcmp(op, "sub") == 0) {return lval_num(x.num - y.num);}
	if (strcmp(op, "mul") == 0) {return lval_num(x.num * y.num);}
	if (strcmp(op, "exp") == 0) {return lval_num(powf(x.num,y.num));}
	if (strcmp(op, "div") == 0)
	{
		return y.num == 0 ? lval_err(LERR_DIV_ZERO) : lval_num(x.num / y.num);
	}
	if (strcmp(op, "mod") == 0)
	{
		return y.num == 0 ? lval_err(LERR_DIV_ZERO) : lval_num((int)x.num % (int)y.num);
	}
}

lval eval(mpc_ast_t* t)
{
	if(strstr(t->tag, "number")) //Base case, returns numbers
	{
		//Check if there is some error in conversion
		errno = 0;
		float x = atof(t->contents);
		return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
	}

	char* op = t->children[1]->contents; //The operator is alwas the second child

	lval x = eval(t->children[2]); //storing 3rd child in x

	int i = 3;
	while(strstr(t->children[i]->tag, "expr"))
	{
		x = eval_op(x, op, eval(t->children[i]));
		i++;
	}

	return x;
}



int main(int argc, char** argv)
{
	//Creating parsers 
	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Operator = mpc_new("operator");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Lispy = mpc_new("lispy");

	//Defining parsers with language
	mpca_lang(MPCA_LANG_DEFAULT,
		"\
			number : /-?[0-9]+\.?[0-9]*/; \
			operator : /add/ | /sub/ | /mul/ | /div/ | /mod/ | /exp/; \
			expr : <number> | '(' <operator> <expr>+ ')' ; \
			lispy : /^/ <operator> <expr>+ /$/; \
		",
		Number, Operator, Expr, Lispy);

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
			//On Sucess print the AST
			lval result = eval(r.output);
			lval_println(result);
			mpc_ast_delete(r.output);
		}

		else 
		{
			//Otherwise print the error
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}
		
		free(input);
	}
	
	mpc_cleanup(4, Number, Operator, Expr, Lispy);
	return 0;
}