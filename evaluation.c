#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpc.h"
#include <editline/readline.h>
long eval_op(long x, char* op, long y)
{
	if (strcmp(op, "add") == 0) {return x+y;}
	if (strcmp(op, "sub") == 0) {return x-y;}
	if (strcmp(op, "mul") == 0) {return x*y;}
	if (strcmp(op, "div") == 0) {return x/y;}
	if (strcmp(op, "mod") == 0) {return x%y;}
	//if (strcmp(op, "exp") == 0) {return powf(x,y);}
	return 0.0;
}

long eval(mpc_ast_t* t)
{
	if(strstr(t->tag, "number")) //Base case, returns numbers
	{
		return atof(t->contents);
	}

	char* op = t->children[1]->contents; //The operator is alwas the second child

	long x = eval(t->children[2]); //storing 3rd child in x

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
			number : /-?[0-9]+\.?[0-9]+/ ; \
			operator : /add/ | /sub/ | /mul/ | /div/ | /mod/ | /exp/; \
			expr : <number> | '(' <operator> <expr>+ ')' ; \
			lispy : /^/ <operator> <expr>+ /$/ ; \
		",
		Number, Operator, Expr, Lispy);

	puts("Lispy Version 0.0.0.4");
	puts("Now with evaluation");
	puts("All numbers must be in decimal form")

	while (1)
	{
		//Output prompt to the user and get input and add it to history
		char* input = readline("lispy> ");
		add_history(input);
		mpc_result_t r;
		if (mpc_parse("<stdin>", input, Lispy, &r))
		{
			//On Sucess print the AST
			float result = eval(r.output);
			printf("%f\n", result);
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