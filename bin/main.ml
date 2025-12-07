open Parser_learning.Simppl_lexer
open Parser_learning.Simppl_parser
open Parser_learning.Ast
open Parser_learning.Semantic
open Parser_learning.Interpreter
open Sexplib
open Format

let fake_env = [("N", ar_singleton 8); ("is", index_from_array [|1;3;2;5;4;8;6;7|])]

let parse_buf buf =
  let tree = filespec read buf in
    fprintf std_formatter "%a@." Sexp.pp_hum (sexp_of_model tree);
    check_model tree;
    let fg = eval_model fake_env tree.params_block tree.model_block in
      ignore (List.map (fun (_, ps) -> print_endline (String.concat ", " ps)) fg)

let () = 
  let file = Core.In_channel.create "./test_spec" in
    let () = parse_buf (Lexing.from_channel file) in
      Core.In_channel.close file