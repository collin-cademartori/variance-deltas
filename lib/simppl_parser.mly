%token TILDE
%token <string>VAR
%token <int>INDEX
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
%token <Ast.datatype>DTYPE
%token COLON
%token FOR
%token IN
%token LEAF
%token ROOT
%token TBD

%start <Ast.meta Ast.model> filespec
%%

block(DEC, spec):
  | DEC; LBRACK; xs = spec; RBRACK; { xs }
  ;

filespec:
  | EOF {{ Ast.data_block = []; Ast.params_block = []; Ast.model_block = []; Ast.tree_block = []; }}
  | ds = option(block(DBD, dataspec));
    ps = block(PBD, paramspec);
    ms = block(MBD, modelspec);
    ts = block(TBD, treespec)
    {{
      Ast.data_block = Option.value ds ~default:[];
      Ast.params_block = ps; 
      Ast.model_block = ms;
      Ast.tree_block = ts;
    }}
  ;

dataspec:
  | ds = list(data_dec) { ds }
  ;

data_dec:
  | dt = DTYPE;
    ils = option(index_list(index_exp));
    dn = VAR 
    { Ast.Data (dn, dt, Option.value ils ~default:[], $loc) }
  ;

paramspec:
  | ps = nonempty_list(param_dec) { ps }
  ;

param_dec:
  | pt = DTYPE; 
    ils = option(index_list(index_exp));
    pn = VAR
    { Ast.Param (pn, pt, Option.value ils ~default:[], $loc) }
  ;

modelspec:
  | vs = nonempty_list(sampling_stmt) { vs }
  ;

sampling_stmt:
  | var = arg; TILDE; dist = VAR; LPAREN; args = argslist; RPAREN
    { Ast.Dist (dist, var, args, $loc) }
  | FOR; LPAREN; loop_i = VAR; IN; loop_is = index_exp; RPAREN;
    LBRACK; loop_spec = modelspec; RBRACK;
    { Ast.For (loop_i, loop_is, loop_spec, $loc) }
  ;

treespec:
  | vs = nonempty_list(tree_stmt) { vs }
  ;

tree_stmt:
  | ROOT; LPAREN; var = arg; RPAREN { Ast.Root (var, $loc) }
  | LEAF; LPAREN; vars = argslist; RPAREN { Ast.Leaf (vars, $loc) }
  | FOR; LPAREN; loop_i = VAR; IN; loop_is = index_exp; RPAREN;
    LBRACK; loop_spec = treespec; RBRACK;
    { Ast.TreeFor (loop_i, loop_is, loop_spec, $loc) }
  ;

argslist:
  | args = separated_nonempty_list(COMMA, arg) { args }
  ;

arg:
  | iarg = INDEX { Ast.LitInt (iarg, $loc) }
  | farg = ARG { Ast.Lit (farg, $loc) }
  | varg = VAR; il = option(index_list(index_exp)) { Ast.Var (varg, Option.value il ~default:[], $loc) }
  ;

index_list(exp):
  | LIND; il = separated_list(COMMA, exp); RIND { il }
  ;

index_exp:
  | ind1 = arg; COLON; ind2 = arg { Ast.Range (ind1, ind2, $loc) }
  | aind = arg; { aind }
  ;