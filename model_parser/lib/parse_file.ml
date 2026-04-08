(** Basic utilities for lexing and parsing a model file, with
    helpers for printing useful error messages with text context. *)

open Simppl_lexer
open Simppl_parser
open Simppl_errors
open MenhirLib.ErrorReports
open Lexing
module LU = MenhirLib.LexerUtil

exception ParseError of string

(** Given a pair of text positions [(pos_begin, pos_end)], create a formatted
    string highlighting the corresponding sequence in the string [text] for use
    in constructing human-readable error messages.  *)
let expand_code text = function
  | (pos_begin, pos_end) -> 
    let err_offset = pos_begin.pos_cnum - pos_end.pos_bol in
    let err_len = pos_end.pos_cnum - pos_begin.pos_cnum in
    let code_str = String.sub text pos_begin.pos_bol err_offset
      ^ String.sub text pos_begin.pos_cnum err_len in
    code_str ^ "\n" ^ (String.make err_offset ' ') ^ (String.make err_len '^')

(** Parse a lexical buffer [buf] into an AST. *)
let parse_buf buf code_text = 
  let pos_buf, lexer = wrap read in
  try
    filespec lexer buf
  with
  | Error ecode -> begin
    let err_line = LU.range (last pos_buf) in
    let err_code = expand_code code_text (last pos_buf) in
    raise (ParseError 
      ("\nParsing error:\n" ^ err_line ^ err_code ^ "\n" ^ (message ecode))
    )
  end

(** Parse a model file with name [filename], returning the parsed AST and raw text of the file. *)
let parse_model file_name =
  let text, lbuf = LU.read file_name in
    lbuf.lex_curr_p <- { lbuf.lex_curr_p with Lexing.pos_fname = file_name };
    (parse_buf lbuf text, text)