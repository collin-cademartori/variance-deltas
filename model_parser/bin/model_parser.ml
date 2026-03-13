open Fg_parser.Ast
open Fg_parser.Semantic
open Fg_parser.Interpreter
open Fg_parser.Read_data
open Lexing
open Format
open Yojson.Basic
open Fg_parser.Parse_file

(* CLI Processing Stuff *)

let correct_usage_message = "parse-fg-spec <spec_file> -d <data_file>"

let spec_file = ref ""
let data_file = ref ""
let set_file file = spec_file := file

let flag_list = [
  ("-d", Arg.Set_string data_file, "Set the data file name");
]

let () = Arg.parse flag_list set_file correct_usage_message

(* Run Parser and Print Formatted Output *)

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

let () = 
  let file_name = !data_file in try (*"./test_data.json"*)
  let data_json = from_file file_name in
  let tree, text = parse_model !spec_file in (*"./test_spec"*)
    try begin
      if print_tree then fprintf std_formatter "%a@." Sexplib0.Sexp.pp_hum (sexp_of_model sexp_of_meta tree);
      check_model tree;
      let data_env = parse_data tree.data_block data_json in
      let fg = eval_model data_env tree.data_block tree.params_block tree.model_block in
        ignore (List.map (fun (dname, ps) -> print_endline ((String.concat "\n" (dname :: ps)) ^ "\n-")) fg);
      print_endline("--");
      let tree = eval_tree data_env tree.params_block tree.tree_block in
        let () = print_endline tree.root in
          ignore (List.map (fun l_names -> print_endline (String.concat ", " l_names)) tree.leaves);
    end with
      | TypeError (msg, loc) -> print_err msg loc text
      | RuntimeError (msg, loc) -> print_err msg loc text
      | DataError (msg, loc) -> print_err msg loc text
  with 
    | ParseError err -> print_endline err;
    | Yojson.Json_error msg ->
        print_endline ("\nError while parsing input JSON file " ^ file_name ^ ": " ^ msg)