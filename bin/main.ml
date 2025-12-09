open Parser_learning.Ast
open Parser_learning.Semantic
open Parser_learning.Interpreter
open Parser_learning.Read_data
open Sexplib
open Format
open Yojson.Basic
open Parser_learning.Parse_file

let data_json = from_file "./test_data.json"

(* exception ParseError of string *)

(* let parse_buf buf = try begin
  let tree = filespec read buf in
    fprintf std_formatter "%a@." Sexp.pp_hum (sexp_of_model tree);
    check_model tree;
    let n_env = (parse_datum data_json [] (List.hd tree.data_block)) in
    let fake_env = parse_datum data_json n_env (List.nth tree.data_block 1) in
    let fg = eval_model fake_env tree.params_block tree.model_block in
      ignore (List.map (fun (_, ps) -> print_endline (String.concat ", " ps)) fg)
  end with Error ecode -> 
      print_endline ("Parsing failed with error: " ^ message ecode) *)

(* let () = 
  let file = Core.In_channel.create "./test_spec" in
    let () = parse_buf (Lexing.from_channel file) in
      Core.In_channel.close file *)

let () = try begin
      let tree = parse_model "./test_spec" in
      fprintf std_formatter "%a@." Sexp.pp_hum (sexp_of_model tree);
      check_model tree;
      let n_env = (parse_datum data_json [] (List.hd tree.data_block)) in
      let fake_env = parse_datum data_json n_env (List.nth tree.data_block 1) in
      let fg = eval_model fake_env tree.params_block tree.model_block in
        ignore (List.map (fun (_, ps) -> print_endline (String.concat ", " ps)) fg)
  end with
  | ParseError err -> print_endline err