open Parser_learning.Simppl_lexer
open Parser_learning.Simppl_parser
open Parser_learning.Ast
(* open Sexplib.Std *)
(* open Lexing *)

let parse_string s =
  let lexbuf = Lexing.from_string s in
  spec read lexbuf

let test_spec =
  match parse_string "theta ~ normal(0,1)" with
  | None -> "Nothing parsed."
  | Some pexp -> Base.Sexp.to_string (sexp_of_fact pexp)

let () = print_endline test_spec
