%token TILDE
%token <string>VAR
%token EOF

%start <Ast.fact option> spec
%%

spec:
  | EOF { None }
  | v = stmt { Some v }
  ;

stmt:
  | var = VAR; TILDE; dist = VAR
    { Ast.Dist (dist, var) } 
  ;