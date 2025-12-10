open Parser_learning.Ast
open Parser_learning.Semantic
open Parser_learning.Interpreter
open Parser_learning.Read_data
open Lexing
open Sexplib
open Format
open Yojson.Basic
open Parser_learning.Parse_file

let data_json = from_file "./test_data.json"

let print_tree = false

let print_err err_msg (err_st, err_en) code_text =
  let code_str = String.sub code_text err_st.pos_cnum (err_en.pos_cnum - err_st.pos_cnum) in
  let total_err = 
    "\nError occurred on line " 
    ^ (string_of_int err_st.pos_lnum)
    ^ " at: \""
    ^ code_str
    ^ "\"\n"
    ^ err_msg ^ "\n"
  in print_endline total_err

let () = try
  let tree, text =  parse_model "./test_spec" in
    try begin
      if print_tree then fprintf std_formatter "%a@." Sexp.pp_hum (sexp_of_model sexp_of_meta tree);
      check_model tree;
      let n_env = (parse_datum data_json [] (List.hd tree.data_block)) in
      let fake_env = parse_datum data_json n_env (List.nth tree.data_block 1) in
      let fg = eval_model fake_env tree.params_block tree.model_block in
        ignore (List.map (fun (_, ps) -> print_endline (String.concat ", " ps)) fg)
    end with
    | TypeError (msg, loc) -> print_err msg loc text
    | RuntimeError (msg, loc) -> print_err msg loc text
  with ParseError err -> print_endline err;