open Simppl_lexer
open Simppl_parser
open Simppl_errors
open MenhirLib.ErrorReports
open Lexing
module LU = MenhirLib.LexerUtil

exception ParseError of string

(* let expand_code text positions =
  extract text positions
  |> sanitize
  |> compress
  |> shorten 20 *)

let expand_code text = function
  | (pos_begin, pos_end) -> 
    let err_offset = pos_begin.pos_cnum - pos_end.pos_bol in
    let err_len = pos_end.pos_cnum - pos_begin.pos_cnum in
    let code_str = String.sub text pos_begin.pos_bol err_offset
      ^ String.sub text pos_begin.pos_cnum err_len in
    code_str ^ "\n" ^ (String.make err_offset ' ') ^ (String.make err_len '^')
    (* begin 
    if (pos_begin.pos_bol = pos_end.pos_bol)
      then String.sub text pos_begin.pos_bol (pos_end.pos_cnum - pos_end.pos_bol)
      else (String.sub text pos_begin.pos_bol (pos_end.pos_bol-1-pos_begin.pos_bol)) 
            ^ "\n" ^ (String.sub text pos_end.pos_bol (pos_end.pos_cnum - pos_end.pos_bol))
    end *)

(* let err_pos_string code_text pos_pair = 
  expand_code code_text pos_pair
  (LU.range pos_pair) ^ (expand_code code_text pos_pair) *)

let parse_buf buf code_text = 
  let pos_buf, lexer = wrap read in
  try
    filespec lexer buf
  with
  | Error ecode -> begin
    let err_line = LU.range (last pos_buf) in
    (* let err_code = show (expand_code code_text) pos_buf in *)
    (* let err_code = show (err_pos_string code_text) pos_buf in *)
    let err_code = expand_code code_text (last pos_buf) in
    raise (ParseError 
      ("\nParsing error:\n" ^ err_line ^ err_code ^ "\n" ^ (message ecode))
    )
  end

let parse_model file_name =
  let text, lbuf = LU.read file_name in
  (* let file = Core.In_channel.create file_name in
    let lbuf = Lexing.from_channel file in *)
      lbuf.lex_curr_p <- { lbuf.lex_curr_p with Lexing.pos_fname = file_name };
      parse_buf lbuf text