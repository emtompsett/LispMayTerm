#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"

#include <editline/readline.h>


int main(int argc, char** argv)
{
	//Creating parsers 
	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Operator = mpc_new("operator");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Lispy = mpc_new("lispy");

	//Defining parsers with language
	mpca_lang(MPCA_LANG_DEFAULT,
		" \
			number : /-?[0-9]+\.?[0-9]*/ ; \
			operator : /add/ | /sub/ | /mul/ | /div/ | /mod/ ; \
			expr : <number> | '(' <operator> <expr>+ ')' ; \
			lispy : /^/ <operator> <expr>+ /$/ ; \
		",
		Number, Operator, Expr, Lispy);

	puts("Lispy Version 0.0.0.3");
	puts("Now with parsers");

	while (1)
	{
		//Output prompt to the user and get input and add it to history
		char* input = readline("lispy> ");
		add_history(input);
		mpc_result_t r;
		if (mpc_parse("<stdin>", input, Lispy, &r))
		{
			//On Sucess print the AST
			mpc_ast_print(r.output);
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