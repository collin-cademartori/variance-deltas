open! Core

open Static_env.StaticEnv

type func_t = Add | Max | Min
[@@deriving sexp]

let func_exp = [
  ("add", Add);
  ("max", Max);
  ("min", Min);
]

let func_types = [
  (Add, ([Int_T; Int_T], Int_T));
  (Max, ([Int_T; Int_T], Int_T));
  (Min, ([Int_T; Int_T], Int_T))
]