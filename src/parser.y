%code requires{
  #include "ast.hpp"
  #include <vector>
  #include <cassert>

  extern const Node *g_root; // A way of getting the AST out

  int yylex(void);
  void yyerror(const char *);
}

// Represents the value associated with any kind of
// AST node.
%union{
  const Node *expr;
  int integer;
  std::string *string;
}


/////////////////////////////////////////////////

%token MULT DIV PLUS SUB LT EQ
%token T_IF T_ELSE T_WHILE  T_END T_PRINT T_BEGIN
%token T_INT T_STRING ASSIGN

%type <expr> FACTOR STATEMENT DECLARATION GLOBAL_DECLARATION_LIST  COMPOUND_STATEMENT SEQ SEQ_PROG
%type <expr> CONDITIONAL_STATEMENT PARAMETER_LIST PARAMETER EXPR_LIST GLOBAL_DECLARATION GLOBAL_VARIABLE_DECLARATION
%type <expr> INPUT_PARAMS  GBL_INIT_PARAMS
%type <expr> ASSIGN_EXPR    EQUALITY_EXPR RELATIONAL_EXPR ADDITIVE_EXPR MULTIPLICATIVE_EXPR DECLARATION_LIST
%type <integer> T_INT


%type <string> T_STRING  T_IF T_ELSE T_WHILE  T_END T_BEGIN T_PRINT
%type <string> EQ SUB PLUS DIV MULT LT ASSIGN



%start ROOT

%%


ROOT : SEQ_PROG { g_root = $1;}

SEQ_PROG
        :  GLOBAL_DECLARATION { $$ = new Sequence(nullptr,$1);}
        | SEQ_PROG GLOBAL_DECLARATION { $$ = new Sequence($1,$2);}

GLOBAL_DECLARATION
        : GLOBAL_VARIABLE_DECLARATION
        | PARAMETER_LIST

PARAMETER_LIST
        :  PARAMETER { $$ = new ParameterList( nullptr,$1);}
        | PARAMETER_LIST PARAMETER { $$ = new ParameterList($1,$2);}


PARAMETER
        :  T_STRING { $$ = new Parameter( nullptr, *$1);}

COMPOUND_STATEMENT
        :  SEQ  { $$ = new CompoundStat($1);}

SEQ
        : SEQ STATEMENT { $$ = new Sequence($1,$2);}
        | STATEMENT { $$ = new Sequence(nullptr,$1);}

STATEMENT
        : EXPR_LIST  { $$ = new Stat($1);}
        | CONDITIONAL_STATEMENT
        | T_PRINT ASSIGN_EXPR  { $$ = new PrintStat($2);}
        | DECLARATION
        | COMPOUND_STATEMENT


CONDITIONAL_STATEMENT
        : T_WHILE  ASSIGN_EXPR T_BEGIN COMPOUND_STATEMENT T_END { $$ = new whileStat( $2, $4 ); }
        | T_IF  ASSIGN_EXPR T_BEGIN STATEMENT T_END { $$ = new ifStat( $2, new CompoundStat( new Sequence( nullptr, $4 )) ); }
        | T_IF  ASSIGN_EXPR  T_BEGIN STATEMENT T_ELSE STATEMENT T_END { $$ = new ifElseStat( $2, $4, $6 ); }


EXPR_LIST
        : ASSIGN_EXPR
        | EXPR_LIST ASSIGN_EXPR { $$ = new ExprList($1,$2);}

ASSIGN_EXPR
        : EQUALITY_EXPR
        | DECLARATION
        | T_STRING ASSIGN EQUALITY_EXPR { $$ = new AssignOp(*$1,nullptr,$3);}

EQUALITY_EXPR
        : RELATIONAL_EXPR
        | EQUALITY_EXPR '=' RELATIONAL_EXPR { $$ = new EqualsOp($1, $3); }

RELATIONAL_EXPR
        : ADDITIVE_EXPR
        | RELATIONAL_EXPR '<' ADDITIVE_EXPR { $$ = new LessOp($1, $3); }


ADDITIVE_EXPR
        : MULTIPLICATIVE_EXPR
        | ADDITIVE_EXPR '+' MULTIPLICATIVE_EXPR {$$= new AddOp($1,$3);}
        | ADDITIVE_EXPR '-' MULTIPLICATIVE_EXPR {$$= new SubOp($1,$3);}

MULTIPLICATIVE_EXPR
        : FACTOR
        | MULTIPLICATIVE_EXPR '*' FACTOR { $$ = new MulOp($1, $3); }
        | MULTIPLICATIVE_EXPR '/' FACTOR { $$ = new DivOp($1, $3); }

FACTOR
        : T_INT          {$$ = new Number( $1 );}
        | T_STRING          {$$ = new Variable(*$1);}



/////////////////////////////////////////////////////////////////////////////////

GLOBAL_VARIABLE_DECLARATION
        :  GLOBAL_DECLARATION_LIST { $$ = new VariableDecl( nullptr, $1);}

GLOBAL_DECLARATION_LIST
        : T_STRING  { $$ = new GlobalDeclList(nullptr, *$1 );}
        | T_STRING  DECLARATION_LIST { $$ = new GlobalDeclList($2, *$1);}
        | T_STRING ASSIGN T_INT { $$ = new GlobalDeclList2(nullptr, *$1, $3 );}
        | T_STRING ASSIGN T_INT  DECLARATION_LIST { $$ = new GlobalDeclList2($4, *$1, $3);}


DECLARATION
        :  DECLARATION_LIST    { $$ = new VariableDecl( nullptr, $1 );}

DECLARATION_LIST
        : T_STRING  { $$ = new DeclList(nullptr, *$1, nullptr);}
        | T_STRING  DECLARATION_LIST { $$ = new DeclList($2, *$1, nullptr);}
        | T_STRING ASSIGN EXPR_LIST  { $$ = new DeclList(nullptr, *$1, $3);}
        | T_STRING ASSIGN EXPR_LIST  DECLARATION_LIST { $$ = new DeclList($4, *$1, $3);}


%%


const Node *g_root;
const Node *parseAST()
{
  g_root=0;
  yyparse();
  return g_root;
}
