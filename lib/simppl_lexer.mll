{
  open Simppl_parser

  exception SyntaxError of string
}

let white = [' ' '\t']+
let newline = '\r' | '\n' | "\r\n"
let varname = ['a'-'z' 'A'-'Z' '_'] ['a'-'z' 'A'-'Z' '0'-'9' '_']*

rule read =
  parse
  | white { read lexbuf }
  | newline { Lexing.new_line lexbuf; read lexbuf }
  | '~' { TILDE }
  | varname { VAR (Lexing.lexeme lexbuf) }
  | _ { 
      raise (SyntaxError 
        ("Unexpected character while lexing: " ^ Lexing.lexeme lexbuf)
      )
  }
  | eof { EOF }