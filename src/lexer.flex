%option noyywrap

%{
	#include "parser.tab.hpp"
	#include <string>
	#include <cstdlib>
	void col_inc();
	void store(char * yytext);

 extern "C" int fileno(FILE *stream);
%}

digit       [0-9]
word        [a-zA-Z]+



%%

begin           { store(yytext); return T_BEGIN; }
end             { store(yytext); return T_END; }
while           { store(yytext); return T_WHILE; }
if              { store(yytext); return T_IF; }
else           	{ store(yytext); return T_ELSE; }
print						{ store(yytext); return T_PRINT; }



"="							{ store(yytext); return EQ; }
"*"             { store(yytext); return MULT; }
"+"             { store(yytext); return PLUS; }
"-"             { store(yytext); return SUB; }
"/"             { store(yytext); return DIV; }
"<"   					{ store(yytext); return LT; }

":="  					{ store(yytext); return ASSIGN; }




{word}*        	{ store(yytext); return T_STRING; }
{digit}*        { yylval.integer= strtod(yytext,0); return T_INT; }

[ \t\r\n]+			{;}

%%

void store(char * yytext)
{yylval.string= new std::string(yytext);}


void yyerror (char const *s)
{
  fprintf (stderr, "Flex Error: %s\n", s); /* s is the text that wasn't matched */
  exit(1);
}
