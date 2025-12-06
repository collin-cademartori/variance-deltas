open Parser_learning.Simppl_lexer
open Parser_learning.Simppl_parser
open Parser_learning.Ast
open Parser_learning.Semantic
open Sexplib
open Format

let parse_buf buf =
  let tree = filespec read buf in
    fprintf std_formatter "%a@." Sexp.pp_hum (sexp_of_model tree);
    check_model tree

let () = 
  let file = Core.In_channel.create "./test_spec" in
    let () = parse_buf (Lexing.from_channel file) in
      Core.In_channel.close file