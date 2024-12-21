// pl0 compiler source code

#pragma warning(disable:4996)


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "PL0.h"
#include "set.c"

//////////////////////////////////////////////////////////////////////
// print error message.
void error(int n)
{
	int i;

	printf("      ");
	for (i = 1; i <= cc - 1; i++)
		printf(" ");
	printf("^\n");
	printf("Error %3d: %s\n", n, err_msg[n]);
	err++;
} // error

//////////////////////////////////////////////////////////////////////
void getch(void)
{
	if (cc == ll)
	{
		if (feof(infile))
		{
			printf("\nPROGRAM INCOMPLETE\n");
			exit(1);
		}
		ll = cc = 0;
		printf("%5d  ", cx);
		while ( (!feof(infile)) // added & modified by alex 01-02-09
			    && ((ch = getc(infile)) != '\n'))
		{
			printf("%c", ch);
			line[++ll] = ch;
		} // while
		printf("\n");
		line[++ll] = ' ';
	}
	ch = line[++cc];
} // getch

//////////////////////////////////////////////////////////////////////
// gets a symbol from input stream.
void getsym(void)
{
	int i, k;
	char a[MAXIDLEN + 1];

	while (ch == ' '||ch == '\t')
		getch();

	if (isalpha(ch))
	{ // symbol is a reserved word or an identifier.
		k = 0;
		do
		{
			if (k < MAXIDLEN)
				a[k++] = ch;
			getch();
		}
		while (isalpha(ch) || isdigit(ch));
		a[k] = 0;
		strcpy(id, a);
		word[0] = id;
		i = NRW;
		while (strcmp(id, word[i--]));
		if (++i)
			sym = wsym[i]; // symbol is a reserved word
		else
			sym = SYM_IDENTIFIER;   // symbol is an identifier
	}
	else if (isdigit(ch))
	{ // symbol is a number.
		k = num = 0;
		sym = SYM_NUMBER;
		do
		{
			num = num * 10 + ch - '0';
			k++;
			getch();
		}
		while (isdigit(ch));
		if (k > MAXNUMLEN)
			error(25);     // The number is too great.
	}
	else if (ch == ':')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_BECOMES; // :=
			getch();
		}
		else
		{
			sym = SYM_COLON;       // 
		}
	}
	else if (ch == '>')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_GEQ;     // >=
			getch();
		}
		else
		{
			sym = SYM_GTR;     // >
		}
	}
	else if (ch == '<')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_LEQ;     // <=
			getch();
		}
		else if (ch == '>')
		{
			sym = SYM_NEQ;     // <>
			getch();
		}
		else
		{
			sym = SYM_LES;     // <
		}
	}
	else
	{ // other tokens
		i = NSYM;
		csym[0] = ch;
		while (csym[i--] != ch);
		if (++i)
		{
			sym = ssym[i];
			getch();
		}
		else
		{
			printf("Fatal Error: Unknown character.\n");
			exit(1);
		}
	}
} // getsym

//////////////////////////////////////////////////////////////////////
// generates (assembles) an instruction.
void gen(int x, int y, int z)
{
	if (cx > CXMAX)
	{
		printf("Fatal Error: Program too long.\n");
		exit(1);
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx++].a = z;
} // gen

//////////////////////////////////////////////////////////////////////
// tests if error occurs and skips all symbols that do not belongs to s1 or s2.
void test(symset s1, symset s2, int n)
{
	symset s;

	if (! inset(sym, s1))
	{
		error(n);
		s = uniteset(s1, s2);
		while(! inset(sym, s))
			getsym();
		destroyset(s);
	}
} // test

//////////////////////////////////////////////////////////////////////
int dx;  // data allocation index

// enter object(constant, variable or procedre) into table.
void enter(int kind)
{
	mask* mk;

	tx++;
	strcpy(table[tx].name, id);
	table[tx].kind = kind;
	switch (kind)
	{
	case ID_CONSTANT:
		if (num > MAXADDRESS)
		{
			error(25); // The number is too great.
			num = 0;
		}
		table[tx].value = num;
		break;
	case ID_VARIABLE:
		mk = (mask*) &table[tx];
		mk->level = level;
		mk->address = dx++;
		break;
	case ID_PROCEDURE:
		mk = (mask*) &table[tx];
		mk->level = level;
		break;
	} // switch
} // enter

//////////////////////////////////////////////////////////////////////
// locates identifier in symbol table.
int position(char* id)
{
	int i;
	strcpy(table[0].name, id);
	i = tx + 1;
	while (strcmp(table[--i].name, id) != 0);
	return i;
} // position

//////////////////////////////////////////////////////////////////////
void constdeclaration()
{
	if (sym == SYM_IDENTIFIER)
	{
		getsym();
		if (sym == SYM_EQU || sym == SYM_BECOMES)
		{
			if (sym == SYM_BECOMES)
				error(1); // Found ':=' when expecting '='.
			getsym();
			if (sym == SYM_NUMBER)
			{
				enter(ID_CONSTANT);
				getsym();
			}
			else
			{
				error(2); // There must be a number to follow '='.
			}
		}
		else
		{
			error(3); // There must be an '=' to follow the identifier.
		}
	} else	error(4);
	 // There must be an identifier to follow 'const', 'var', or 'procedure'.
} // constdeclaration

//////////////////////////////////////////////////////////////////////
void vardeclaration(void)
{
	if (sym == SYM_IDENTIFIER)
	{
		enter(ID_VARIABLE);
		getsym();
	}
	else
	{
		error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
	}
} // vardeclaration

//////////////////////////////////////////////////////////////////////
void listcode(int from, int to)
{
	int i;
	
	printf("\n");
	for (i = from; i < to; i++)
	{
		printf("%5d %s\t%d\t%d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
	}
	printf("\n");
} // listcode

//////////////////////////////////////////////////////////////////////
void factor(symset fsys)
{
	int expression(symset fsys);
	int i;
	symset set;
	
	test(facbegsys, fsys, 24); // The symbol can not be as the beginning of an expression.

	if (inset(sym, facbegsys))
	{
		if (sym == SYM_IDENTIFIER)
		{
			if ((i = position(id)) == 0)
			{
				error(11); // Undeclared identifier.
			}
			else
			{
				switch (table[i].kind)
				{
					mask* mk;
				case ID_CONSTANT:
					gen(LIT, 0, table[i].value);
					break;
				case ID_VARIABLE:
					mk = (mask*) &table[i];
					gen(LOD, level - mk->level, mk->address);
					break;
				case ID_PROCEDURE:
					error(21); // Procedure identifier can not be in an expression.
					break;
				} // switch
			}
			getsym();
		}
		else if (sym == SYM_NUMBER)
		{
			if (num > MAXADDRESS)
			{
				error(25); // The number is too great.
				num = 0;
			}
			gen(LIT, 0, num);
			getsym();
		}
		else if (sym == SYM_LPAREN)
		{
			getsym();
			set = uniteset(createset(SYM_RPAREN, SYM_NULL), fsys);
			expression(set);
			destroyset(set);
			if (sym == SYM_RPAREN)
			{
				getsym();
			}
			else
			{
				error(22); // Missing ')'.
			}
		}
		else if(sym == SYM_MINUS) // UMINUS,  Expr -> '-' Expr
		{  
			 getsym();
			 factor(fsys);
			 gen(OPR, 0, OPR_NEG);
		}
		test(fsys, createset(SYM_LPAREN, SYM_NULL), 23);
	} // if
} // factor

//////////////////////////////////////////////////////////////////////
void term(symset fsys)
{
	int mulop;
	symset set;
	
	set = uniteset(fsys, createset(SYM_TIMES, SYM_SLASH, SYM_NULL));
	factor(set);
	while (sym == SYM_TIMES || sym == SYM_SLASH)
	{
		mulop = sym;
		getsym();
		factor(set);
		if (mulop == SYM_TIMES)
		{
			gen(OPR, 0, OPR_MUL);
		}
		else
		{
			gen(OPR, 0, OPR_DIV);
		}
	} // while
	destroyset(set);
} // term

//////////////////////////////////////////////////////////////////////
int expression(symset fsys)
{
	int addop;
	symset set;
        int result;
	set = uniteset(fsys, createset(SYM_PLUS, SYM_MINUS, SYM_NULL));
	
	term(set);
	while (sym == SYM_PLUS || sym == SYM_MINUS)
	{
		addop = sym;
		getsym();
		term(set);
		if (addop == SYM_PLUS)
		{
			gen(OPR, 0, OPR_ADD);
		}
		else
		{
			gen(OPR, 0, OPR_MIN);
		}
	} // while

	destroyset(set);
	return result;
} // expression

//////////////////////////////////////////////////////////////////////
void condition(symset fsys)
{
	int relop;
	symset set;

	if (sym == SYM_ODD)
	{
		getsym();
		expression(fsys);
		gen(OPR, 0, 6);
	}
	else
	{
		set = uniteset(relset, fsys);
		expression(set);
		destroyset(set);
		if (! inset(sym, relset))
		{
			error(20);
		}
		else
		{
			relop = sym;
			getsym();
			expression(fsys);
			switch (relop)
			{
			case SYM_EQU:
				gen(OPR, 0, OPR_EQU);
				break;
			case SYM_NEQ:
				gen(OPR, 0, OPR_NEQ);
				break;
			case SYM_LES:
				gen(OPR, 0, OPR_LES);
				break;
			case SYM_GEQ:
				gen(OPR, 0, OPR_GEQ);
				break;
			case SYM_GTR:
				gen(OPR, 0, OPR_GTR);
				break;
			case SYM_LEQ:
				gen(OPR, 0, OPR_LEQ);
				break;
			} // switch
		} // else
	} // else
} // condition

//////////////////////////////////////////////////////////////////////
void statement(symset fsys)
{
	int i, cx1, cx2,cx3, cx4;
	symset set1, set,set2;

	if (sym == SYM_IDENTIFIER)
	{ // variable assignment
		mask* mk;
		if (! (i = position(id)))
		{
			error(11); // Undeclared identifier.
		}
		else if (table[i].kind != ID_VARIABLE)
		{
			error(12); // Illegal assignment.
			i = 0;
		}
		getsym();
		if (sym == SYM_BECOMES)
		{
			getsym();
		}
		else
		{
			error(13); // ':=' expected.
		}
		expression(fsys);
		mk = (mask*) &table[i];
		if (i)
		{
			gen(STO, level - mk->level, mk->address);
		}
	}
	else if (sym == SYM_CALL)
	{ // procedure call
		getsym();
		if (sym != SYM_IDENTIFIER)
		{
			error(14); // There must be an identifier to follow the 'call'.
		}
		else
		{
			if (! (i = position(id)))
			{
				error(11); // Undeclared identifier.
			}
			else if (table[i].kind == ID_PROCEDURE)
			{
				mask* mk;
				mk = (mask*) &table[i];
				gen(CAL, level - mk->level, mk->address);
			}
			else
			{
				error(15); // A constant or variable can not be called. 
			}
			getsym();
		}
	} 
	else if (sym == SYM_IF)
	{ // if statement
		getsym();
		set1 = createset(SYM_THEN, SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		condition(set);
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_THEN)
		{
			getsym();
		}
		else
		{
			error(16); // 'then' expected.
		}
		cx1 = cx;
		gen(JPC, 0, 0);
		statement(fsys);
		code[cx1].a = cx;	
	}
	else if (sym == SYM_ELIF) 
    { // elif statement
        getsym();
        set1 = createset(SYM_THEN, SYM_DO, SYM_NULL);
        set = uniteset(set1, fsys);
        condition(set); // Condition expression
        destroyset(set1);
        destroyset(set);
        if (sym == SYM_THEN)
        {
            getsym();
        }
        else
        {
            error(16); // 'then' expected.
        }
        cx1 = cx;
        gen(JPC, 0, 0); // Jump if condition is false
        statement(fsys);
        code[cx1].a = cx; // Patch the jump instruction after elif block
    }
	else if (sym == SYM_ELSE)
    { // else statement
        getsym();
        cx1 = cx;
        gen(JMP, 0, 0); // Jump over the elif/else block
        code[cx2].a = cx; // Patch the earlier JPC
        statement(fsys);
        code[cx1].a = cx; // Patch the jump here to execute after the else block
    }
	else if (sym == SYM_EXIT)
    { // exit statement
        getsym();
        gen(HALT, 0, 0); // Exit the program
    }
	else if (sym == SYM_RETURN)
    { // return statement
        getsym();
        if (sym != SYM_RPAREN)
        {
            error(23); // Missing return value (if any)
        }
        getsym();
        set1 = createset(SYM_RPAREN, SYM_NULL);
        set = uniteset(set1, fsys);
        expression(set, 0); // Process return value expression
        destroyset(set1);
        destroyset(set);
        gen(RET, 0, 0); // Return to caller
    }	
	else if (sym == SYM_BEGIN)
	{ // block
		getsym();
		set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
		set = uniteset(set1, fsys);
		statement(set);
		while (sym == SYM_SEMICOLON || inset(sym, statbegsys))
		{
			if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else
			{
				error(10);
			}
			statement(set);
		} // while
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_END)
		{
			getsym();
		}
		else
		{
			error(17); // ';' or 'end' expected.
		}
	}
	else if (sym == SYM_WHILE)
	{ // while statement
		cstack[ctop].ty = env;                            
		cstack[ctop].c =he; 
		ctop++;                  //进入新循环保留记录上一个环境                     
		env = ENV_WHILE;//更新环境
		he = cx;    
		cx1 = cx;
		getsym();
		set1 = createset(SYM_DO, SYM_NULL);
        set = uniteset(set1, fsys);
        condition(set);
        destroyset(set1);
        destroyset(set);
		cx2 = cx;
		gen(JPC, 0, 0);
		if (sym == SYM_DO)
		{
			getsym();
		}
		else
		{
			error(18); // 'do' expected.
		}
		statement(fsys);
		gen(JMP, 0, cx1);
		code[cx2].a = cx;
		ta = cx;

		int i;
		for (i = ctop - count; i < ctop; i++)
		{
			code[cstack[i].c].a = ta;                                //backpatch break
		}
		ctop = ctop - count - 1;
        count = 0;
        he = cstack[ctop].c;
        env = cstack[ctop].ty;                                         //regain head
	}
	else if (sym == SYM_DO)                                                						//modified by lzp 2017/12/16
	{ // do-while statement
		cstack[ctop].c = he;
		cstack[ctop++].ty = env;
		env = ENV_DO;
		he = cx;
		getsym();
		cx1 = cx; //记录循环开始的位置
		statement(fsys);
		if (sym != SYM_WHILE)
		{
			error(49);                 //missing 'while' in do-while
		}
		else
			getsym();
		if (sym != SYM_LPAREN)
		{
			error(43);                        //missing '('
		}
		else
			getsym();
		ta = cx;  //记录条件判断的位置
		
        set1 = createset(SYM_SEMICOLON, SYM_RPAREN, SYM_NULL);
        set = uniteset(set1, fsys);
        condition(set);
        destroyset(set1);
        destroyset(set);

		
		if (sym == SYM_RPAREN)
		{
			getsym();
		}
		else
		{
			error(22);            //missing ')'
		}
		if (sym == SYM_SEMICOLON)
		{
			getsym();
		}
		else
		{
			error(26);            //missing ';'
		}
		cx2 = cx;        //循环体结束
		gen(JPC, 0, 0);
		gen(JMP, 0, cx1);
		code[cx2].a = cx;
		int i;
		for (i = ctop - count; i < ctop; i++)
		{
			if (cstack[i].ty == CON_BREAK)
			{
				code[cstack[ctop].c].a = cx;
			}
			else if (cstack[i].ty == CON_CONTINUE)
			{
				code[cstack[i].c].a = ta;
			}
		}
		ctop = ctop - count - 1;
        count = 0;
        he = cstack[ctop].c;
        env = cstack[ctop].ty; 
	}//else if
	else if (sym == SYM_BREAK)                                									//added by lzp 17/12/16
	{
		getsym();
		if (sym != SYM_SEMICOLON)
		{
			error(26);                                        //missing ';'
		}
		else
		{
			getsym();
		}
		if (env == ENV_SWITCH)
		{
			gen(JMP, 0, 0);
		}
		else
		{
			count++;
			cstack[ctop].ty = CON_BREAK;                     //store information in the stack
			cstack[ctop++].c = cx;
			gen(JMP, 0, 0);
		}
	}
	else if (sym == SYM_CONTINUE)                              									//added by lzp 17/12/16
	{
		getsym();
		if (sym != SYM_SEMICOLON)
		{
			error(26);                                        //missing ';'
		}
		else
		{
			getsym();
		}
		if (env == ENV_FOR || env == ENV_WHILE)
		{
			gen(JMP, 0, he);                                 //directly jump to the head
		}
		else if (env == ENV_DO)
		{
			count++;
			cstack[ctop].ty = CON_CONTINUE;                       //store necessary information
			cstack[ctop++].c = cx;
			gen(JMP, 0, 0);
		}
	}
	else if (sym == SYM_GOTO)                                   								//added by lzp 17/12/16
	{
		int i;
		getsym();
		if ( (i = position(id)) == 0)
		{
			error(59); // Undeclared label.
			getsym();
		}
		else
		{
			getsym();
		}
		if (sym != SYM_SEMICOLON)
		{
			error(26);                                         //missing ';'
		}
		else
		{
			getsym();
		}
		gen(JMP, 0, table[i].value);                        //junp instruction
	}
	else if (sym == SYM_SWITCH)                                         						//modified by lzp 17/12/16
	{
		cstack[ctop++].ty = env;
		env = ENV_SWITCH;
		getsym();
		if (sym != SYM_RPAREN)
		{
			error(43);           //missing '('
		}//if
		else
		{
			getsym();
		}//else
        set1 = createset(SYM_CASE, SYM_BEGIN, SYM_RPAREN, SYM_BEGIN, SYM_NULL);
        set = uniteset( fsys, set1);
        expression(set);
        destroyset(set1);
        destroyset(set);
		if (sym != SYM_RPAREN)
		{
			error(22);                       //missing ')'
		}//if
		else
		{
			getsym();
		}//else
		if (sym != SYM_BEGIN)
		{
			error(50);              //missing 'begin'
		}//if
		else
		{
			getsym();
		}//else
		if ((sym != SYM_CASE) || (sym != SYM_DEFAULT) || (sym != SYM_END))
		{
			error(51);              //missing 'case','end' or 'default'
		}//if
		int tmp;
		int de_break;         //mark whether there is 'break' after 'default'
		int cx_br;
		int num_case = 0;         //count num of case or default
		cx1 = cx;
		gen(JMP, 0, 0);            //goto test
		while (sym != SYM_END)
		{
			num_case++;
			if (tx_c == maxcase)
			{
				switchtab = (casetab *)realloc(switchtab, sizeof(casetab)*(maxcase + INCREMENT));
				maxcase += INCREMENT;
			}//if         //prepare for more case
			tmp = sym;                                     //store the keyword 'case' or 'default'
			if (sym == SYM_CASE) {
				set = uniteset(fsys, statbegsys);
				setinsert(set, SYM_COLON);
				switchtab[tx_c].t = expression(set);
				destroyset(set);
			}//if
			if (sym != SYM_COLON)
			{
				error(52);              //missing ':'
			}//if
			else
			{
				getsym();
			}//else
			if (tmp != SYM_DEFAULT)
			{
				switchtab[tx_c].c = cx;
			}//if
			else
			{
				cx2 = cx;//default place
			}//else
			while ((sym != SYM_CASE)||(sym != SYM_DEFAULT) ||(sym != SYM_END))       //inside case,default
			{
				if (sym == SYM_BREAK)
				{
					if (tmp != SYM_DEFAULT)
					{
						switchtab[tx_c].flag = TRUE;      //break
						switchtab[tx_c++].cx_bre = cx;
					}//if
					else
					{
						de_break = TRUE;
						cx_br = cx;
					}//else
				
				}//if
				set2 = uniteset(fsys, statbegsys);
				set1=createset(SYM_CASE, SYM_END, SYM_NULL);
				set=uniteset(set1,set2);
				statement(set);
				destroyset(set);
			}//while2
		}//while1
		cx3 = cx;
		gen(JMP, 0, 0); //cx3
		code[cx1].a = cx;                                            //test
		int i;
		for (i = tx_c - num_case; i < tx_c; i++)                       //gen junp ins fo case and default
		{
			gen(JET, switchtab[i].t, switchtab[i].c);
		}
		gen(JMP, 0, cx2);                                           //default ,at the end
		for (i = tx_c - num_case; i < tx_c; i++)                      //backpatch for break
		{
			if (switchtab[i].flag == TRUE)
			{
				code[switchtab[i].cx_bre].a = cx;
			}
		}//for
		if (de_break == TRUE)
		{
			code[cx_br].a = cx;
		}
		code[cx3].a = cx;                                 //if ther is no break ,we can jump out of switch
		tx_c -= num_case;                                //delete case of inside switch stat
		ctop--;
		env = cstack[ctop].ty;
	}
	else if (sym == SYM_FOR)
{ // for statement
	cstack[ctop].c = he;                              									//modified by lzp 17/12/16
	cstack[ctop++].ty = env;
	env = ENV_FOR;
	getsym();
	if (sym != SYM_LPAREN)
	{
		error(43);  //missing '('
	}
	else
	{
		getsym();
	}
	set = uniteset(fsys,statbegsys );													// modified by nanahka 17-12-21
	set1=createset( SYM_SEMICOLON, SYM_RPAREN, SYM_IDENTIFIER, SYM_NULL);
	fsys=uniteset(set1,fsys);
	if (sym == SYM_IDENTIFIER)
	{
		getsym();
		if (! (i = position(id)))
		{
			error(11); // Undeclared identifier.
		}
		else if (table[i].kind == ID_PROCEDURE)
		{
			error(54); // Incorrect type as an lvalue expression.
		}
		else
		{
			assignment(i, set);
		}
	}
	else
	{
		error(44); // There must be a variable in 'for' statement.
	}
	set1 = createset(SYM_SEMICOLON, SYM_NULL);
	test(set1, set, 10); // ';' expected
	if (sym == SYM_SEMICOLON)
	{
		getsym();
	}
	cx1 = cx;
	                                       //modified by lzp 17/12/16
	condition(set);          //condition
	destroyset(set);
	test(set1, set, 10); // ';' expected
	destroyset(set1);
	if (sym == SYM_SEMICOLON)
	{
		getsym();
	}
	cx2 = cx;
	gen(JPC, 0, 0);
	cx3 = cx;
	gen(JMP, 0, 0);
	cx4 = cx;
	he = cx;
	set = uniteset(fsys,statbegsys );													// modified by nanahka 17-12-21
	set1=createset( SYM_SEMICOLON, SYM_RPAREN, SYM_NULL);
	fsys=uniteset(set1,fsys);
	if (sym == SYM_IDENTIFIER)
	{
		getsym();
		if (! (i = position(id)))
		{
			error(11); // Undeclared identifier.
		}
		else if (table[i].kind == ID_PROCEDURE)
		{
			error(54); // Incorrect type as an lvalue expression.
		}
		else
		{
			assignment(i, set);
		}
	}
	else
	{
		error(44); // There must be a variable in 'for' statement.
	}
	destroyset(set);
	set1 = createset(SYM_RPAREN, SYM_NULL);
	test(set1, set, 22); // Missing ')'.
	destroyset(set1);
	if (sym == SYM_RPAREN)
	{
		getsym();
	}
	gen(JMP, 0, cx1);
	code[cx3].a = cx;
	statement(fsys);       //body of 'for'
	gen(JMP, 0, cx4);
	code[cx2].a = cx;
	ta = cx;                                                         						//modified by lzp 17/12/16
	for (i = ctop - count; i < ctop; i++)
	{
		code[cstack[i].c].a = ta;
	}
	ctop= ctop - count - 1;
	count = 0;
	he= cstack[ctop].c;
	env = cstack[ctop].ty;
}
	test(fsys, phi, 19);
} // statement
			
//////////////////////////////////////////////////////////////////////
void block(symset fsys)
{
	int cx0; // initial code index
	mask* mk;
	int block_dx;
	int savedTx;
	symset set1, set;

	dx = 3;
	block_dx = dx;
	mk = (mask*) &table[tx];
	mk->address = cx;
	gen(JMP, 0, 0);
	if (level > MAXLEVEL)
	{
		error(32); // There are too many levels.
	}
	do
	{
		if (sym == SYM_CONST)
		{ // constant declarations
			getsym();
			do
			{
				constdeclaration();
				while (sym == SYM_COMMA)
				{
					getsym();
					constdeclaration();
				}
				if (sym == SYM_SEMICOLON)
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			}
			while (sym == SYM_IDENTIFIER);
		} // if

		if (sym == SYM_VAR)
		{ // variable declarations
			getsym();
			do
			{
				vardeclaration();
				while (sym == SYM_COMMA)
				{
					getsym();
					vardeclaration();
				}
				if (sym == SYM_SEMICOLON)
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			}
			while (sym == SYM_IDENTIFIER);
		} // if
		block_dx = dx; //save dx before handling procedure call!
		while (sym == SYM_PROCEDURE)
		{ // procedure declarations
			getsym();
			if (sym == SYM_IDENTIFIER)
			{
				enter(ID_PROCEDURE);
				getsym();
			}
			else
			{
				error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
			}


			if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else
			{
				error(5); // Missing ',' or ';'.
			}

			level++;
			savedTx = tx;
			set1 = createset(SYM_SEMICOLON, SYM_NULL);
			set = uniteset(set1, fsys);
			block(set);
			destroyset(set1);
			destroyset(set);
			tx = savedTx;
			level--;

			if (sym == SYM_SEMICOLON)
			{
				getsym();
				set1 = createset(SYM_IDENTIFIER, SYM_PROCEDURE, SYM_NULL);
				set = uniteset(statbegsys, set1);
				test(set, fsys, 6);
				destroyset(set1);
				destroyset(set);
			}
			else
			{
				error(5); // Missing ',' or ';'.
			}
		} // while
		dx = block_dx; //restore dx after handling procedure call!
		set1 = createset(SYM_IDENTIFIER, SYM_NULL);
		set = uniteset(statbegsys, set1);
		test(set, declbegsys, 7);
		destroyset(set1);
		destroyset(set);
	}
	while (inset(sym, declbegsys));

	code[mk->address].a = cx;
	mk->address = cx;
	cx0 = cx;
	gen(INT, 0, block_dx);
	set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
	set = uniteset(set1, fsys);
	statement(set);
	destroyset(set1);
	destroyset(set);
	gen(OPR, 0, OPR_RET); // return
	test(fsys, phi, 8); // test for error: Follow the statement is an incorrect symbol.
	listcode(cx0, cx);
} // block

//////////////////////////////////////////////////////////////////////
int base(int stack[], int currentLevel, int levelDiff)
{
	int b = currentLevel;
	
	while (levelDiff--)
		b = stack[b];
	return b;
} // base

//////////////////////////////////////////////////////////////////////
// interprets and executes codes.
void interpret()
{
	int pc;        // program counter
	int stack[STACKSIZE];
	int top;       // top of stack
	int b;         // program, base, and top-stack register
	instruction i; // instruction register

	printf("Begin executing PL/0 program.\n");

	pc = 0;
	b = 1;
	top = 3;
	stack[1] = stack[2] = stack[3] = 0;
	do
	{
		i = code[pc++];
		switch (i.f)
		{
		case LIT:
			stack[++top] = i.a;
			break;
		case OPR:
			switch (i.a) // operator
			{
			case OPR_RET:
				top = b - 1;
				pc = stack[top + 3];
				b = stack[top + 2];
				break;
			case OPR_NEG:
				stack[top] = -stack[top];
				break;
			case OPR_ADD:
				top--;
				stack[top] += stack[top + 1];
				break;
			case OPR_MIN:
				top--;
				stack[top] -= stack[top + 1];
				break;
			case OPR_MUL:
				top--;
				stack[top] *= stack[top + 1];
				break;
			case OPR_DIV:
				top--;
				if (stack[top + 1] == 0)
				{
					fprintf(stderr, "Runtime Error: Divided by zero.\n");
					fprintf(stderr, "Program terminated.\n");
					continue;
				}
				stack[top] /= stack[top + 1];
				break;
			case OPR_ODD:
				stack[top] %= 2;
				break;
			case OPR_EQU:
				top--;
				stack[top] = stack[top] == stack[top + 1];
				break;
			case OPR_NEQ:
				top--;
				stack[top] = stack[top] != stack[top + 1];
				break;
			case OPR_LES:
				top--;
				stack[top] = stack[top] < stack[top + 1];
				break;
			case OPR_GEQ:
				top--;
				stack[top] = stack[top] >= stack[top + 1];
				break;
			case OPR_GTR:
				top--;
				stack[top] = stack[top] > stack[top + 1];
				break;
			case OPR_LEQ:
				top--;
				stack[top] = stack[top] <= stack[top + 1];
				break;
			} // switch
			break;
		case LOD:
			stack[++top] = stack[base(stack, b, i.l) + i.a];
			break;
		case STO:
			stack[base(stack, b, i.l) + i.a] = stack[top];
			printf("%d\n", stack[top]);
			top--;
			break;
		case CAL:
			stack[top + 1] = base(stack, b, i.l);
			// generate new block mark
			stack[top + 2] = b;
			stack[top + 3] = pc;
			b = top + 1;
			pc = i.a;
			break;
		case INT:
			top += i.a;
			break;
		case JMP:
			pc = i.a;
			break;
		case JPC:
			if (stack[top] == 0)
				pc = i.a;
			top--;
			break;
		} // switch
	}
	while (pc);

	printf("End executing PL/0 program.\n");
} // interpret


//////////////////////////////////////////////////////////////////////
void main ()
{
	FILE* hbin;
	char s[80];
	int i;
	symset set, set1, set2;

	printf("Please input source file name: "); // get file name to be compiled
	scanf("%s", s);
	if ((infile = fopen(s, "r")) == NULL)
	{
		printf("File %s can't be opened.\n", s);
		exit(1);
	}

	phi = createset(SYM_NULL);
	relset = createset(SYM_EQU, SYM_NEQ, SYM_LES, SYM_LEQ, SYM_GTR, SYM_GEQ, SYM_NULL);
	
	// create begin symbol sets
	declbegsys = createset(SYM_CONST, SYM_VAR, SYM_PROCEDURE, SYM_NULL);
	statbegsys = createset(SYM_BEGIN, SYM_CALL, SYM_IF, SYM_WHILE, SYM_NULL);
	facbegsys = createset(SYM_IDENTIFIER, SYM_NUMBER, SYM_LPAREN, SYM_MINUS, SYM_NULL);

	err = cc = cx = ll = 0; // initialize global variables
	ch = ' ';
	kk = MAXIDLEN;

	getsym();

	set1 = createset(SYM_PERIOD, SYM_NULL);
	set2 = uniteset(declbegsys, statbegsys);
	set = uniteset(set1, set2);
	block(set);
	destroyset(set1);
	destroyset(set2);
	destroyset(set);
	destroyset(phi);
	destroyset(relset);
	destroyset(declbegsys);
	destroyset(statbegsys);
	destroyset(facbegsys);

	if (sym != SYM_PERIOD)
		error(9); // '.' expected.
	if (err == 0)
	{
		hbin = fopen("hbin.txt", "w");
		for (i = 0; i < cx; i++)
			fwrite(&code[i], sizeof(instruction), 1, hbin);
		fclose(hbin);
	}
	if (err == 0)
		interpret();
	else
		printf("There are %d error(s) in PL/0 program.\n", err);
	listcode(0, cx);
} // main

//////////////////////////////////////////////////////////////////////
// eof pl0.c
