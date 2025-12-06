open! Core

(* type var = Var of string * int list [@@deriving sexp] *)
(* type args = Args of float list [@@deriving sexp] *)

(* type var_index = 
  | Range of arg * arg
  | IndexSet of arg
and arg =
  | LitInt of int
  | VarInt of string * var_index list
  [@@deriving sexp]

type farg =
  | Lit of float
  | Var of string * var_index list
  [@@deriving sexp] *)

type stmt =
  | Lit of float
  | LitInt of int
  | Var of string * stmt list
  | Range of stmt * stmt
  (* | IndexSet of stmt *)
[@@deriving sexp]

type sample_stmt = 
  | Dist of string * stmt * stmt list
  | For of string * stmt * sample_stmt list
  [@@deriving sexp]

type paramtype = Real | Array
[@@deriving sexp]

type param_dec_stmt = Param of string * paramtype * stmt list
[@@deriving sexp]

type datatype = Bool | Int | IArray
[@@deriving sexp]

type data_dec_stmt = Data of string * datatype * stmt list
[@@deriving sexp]

type model = {
  params_block : param_dec_stmt list;
  model_block : sample_stmt list;
  data_block : data_dec_stmt list;
}
[@@deriving sexp]