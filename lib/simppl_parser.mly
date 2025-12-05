%token TILDE
%token <string>VAR
%token <float>ARG 
%token EOF
%token LPAREN
%token RPAREN
%token COMMA
%token LBRACK
%token RBRACK
%token LIND
%token RIND
%token MBD
%token PBD
%token DBD
%token <Ast.paramtype>PTYPE
%token <Ast.datatype>DTYPE
%token <int>INDEX
%token COLON
%token FOR
%token IN

%start <Ast.model> filespec
%%

block(DEC, spec):
  | DEC; LBRACK; xs = spec; RBRACK; { xs }
  ;

filespec:
  | EOF { { Ast.data_block = []; Ast.params_block = []; Ast.model_block = [] } }
  | ds = option(block(DBD, dataspec));
    ps = block(PBD, paramspec);
    ms = block(MBD, modelspec);
    {{
      Ast.data_block = Option.value ds ~default:[];
      Ast.params_block = ps; 
      Ast.model_block = ms
    }}
  ;

dataspec:
  | ds = list(data_dec) { ds }
  ;

data_dec:
  | dt = DTYPE;
    ils = option(index_list(INDEX));
    dn = VAR 
    { Ast.Data (dn, dt, Option.value ils ~default:[]) }
  ;

paramspec:
  | ps = nonempty_list(param_dec) { ps }
  ;

param_dec:
  | pt = PTYPE; 
    ils = option(index_list(INDEX));
    pn = VAR
    { Ast.Param (pn, pt, Option.value ils ~default:[]) }
  ;

modelspec:
  | vs = nonempty_list(sampling_stmt) { vs }
  ;

sampling_stmt:
  | var = arg; TILDE; dist = VAR; LPAREN; args = argslist; RPAREN
    { Ast.Dist (dist, var, args ) }
  | FOR; LPAREN; loop_i = VAR; IN; loop_is = index_exp; RPAREN;
    LBRACK; loop_spec = modelspec; RBRACK;
    { Ast.For (loop_i, loop_is, loop_spec) }
  ;

argslist:
  | args = separated_nonempty_list(COMMA, arg) { args }
  ;

arg:
  | farg = ARG { Ast.Lit farg }
  | varg = VAR; il = option(index_list(index_exp)) { Ast.Var (varg, Option.value il ~default:[]) }
  ;

index_list(exp):
  | LIND; il = separated_list(COMMA, exp); RIND { il }
  ;

index_exp:
  | ind = INDEX { Ast.IndexSet (Ast.LitInt ind) }
  | ind1 = INDEX; COLON; ind2 = INDEX { Ast.Range (Ast.LitInt ind1, Ast.LitInt ind2) }
  | aind = VAR; { Ast.IndexSet (Ast.VarInt (aind, [])) }
  ;