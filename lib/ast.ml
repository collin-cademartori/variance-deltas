open! Core
open Lexing
open Lib_types

type meta = position * position

let sexp_of_meta (st, en) = Sexp.List [
  Sexp.Atom (string_of_int st.pos_cnum);
  Sexp.Atom (string_of_int en.pos_cnum)
]

type datatype = Bool | Int | IArray | Real | Array
[@@deriving sexp]

let dtype_is_integer = function
  | Int -> true
  | IArray -> true
  | _ -> false

type 'm stmt =
  | Lit of float * 'm
  | LitInt of int * 'm
  | Var of string * ('m stmt) list * 'm
  | Range of ('m stmt) * ('m stmt) * 'm
  | Func of func_t * ('m stmt) list * 'm
[@@deriving sexp]

type 'm sample_stmt = 
  | Dist of string * ('m stmt) * ('m stmt) list * 'm
  | For of string * ('m stmt) * ('m sample_stmt) list * 'm
  [@@deriving sexp]

(* type paramtype = Real | Array
[@@deriving sexp] *)

type 'm param_dec_stmt = Param of string * datatype * ('m stmt) list * 'm
[@@deriving sexp]

type 'm data_dec_stmt = Data of string * datatype * ('m stmt) list * 'm
[@@deriving sexp]

type 'm tree_stmt =
  | Leaf of ('m stmt) list * 'm
  | Root of ('m stmt) * 'm
  | TreeFor of string * ('m stmt) * ('m tree_stmt) list * 'm
[@@deriving sexp]

type 'm model = {
  params_block : ('m param_dec_stmt) list;
  model_block : ('m sample_stmt) list;
  data_block : ('m data_dec_stmt) list;
  tree_block : ('m tree_stmt) list;
}
[@@deriving sexp]