open! Sexplib.Std

open Static_env.StaticEnv

type func_t = Add | Max | Min | Sub
[@@deriving sexp]

let func_exp = [
  ("add", Add);
  ("max", Max);
  ("min", Min);
  ("sub", Sub);
]

let func_types = [
  (Add, ([Int_T; Int_T], Int_T));
  (Max, ([Int_T; Int_T], Int_T));
  (Min, ([Int_T; Int_T], Int_T));
  (Sub, ([Int_T; Int_T], Int_T))
]