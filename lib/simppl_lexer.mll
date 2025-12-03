{
  open Simppl_parser

  exception SyntaxError of string
}

let white = [' ' '\t']+
let newline = '\r' | '\n' | "\r\n"
let varname = ['a'-'z' 'A'-'Z' '_'] ['a'-'z' 'A'-'Z' '0'-'9' '_']*
let digit = ['0'-'9']
let int = '-'? digit+
let nat = digit+
let frac = '.' digit+
let dec = int frac?
let model_lit = "model"
let param_lit = "parameters"
let data_lit = "data"
let real_lit = "real"
let array_lit = "array"
let int_lit = "int"
let iarray_lit = "int_array"
let bool_lit = "bool"
let colon_lit = ":"


rule read =
  parse
  | white { read lexbuf }
  | newline { Lexing.new_line lexbuf; read lexbuf }
  | "~" { TILDE }
  | "(" { LPAREN }
  | ")" { RPAREN }
  | "," { COMMA }
  | "{" { LBRACK }
  | "}" { RBRACK }
  | "[" { LIND }
  | "]" { RIND }
  | real_lit { PTYPE Ast.Real }
  | array_lit { PTYPE Ast.Array }
  | int_lit { DTYPE Ast.Int }
  | iarray_lit { DTYPE Ast.IArray }
  | bool_lit { DTYPE Ast.Bool }
  | model_lit { MBD }
  | param_lit { PBD }
  | data_lit { DBD }
  | colon_lit { COLON }
  | nat { INDEX (int_of_string (Lexing.lexeme lexbuf))}
  | dec { ARG (float_of_string (Lexing.lexeme lexbuf))}
  | varname { VAR (Lexing.lexeme lexbuf) }
  | _ 
    { 
      raise (SyntaxError 
        ("Unexpected character while lexing: " ^ Lexing.lexeme lexbuf)
      )
    }
  | eof { EOF }