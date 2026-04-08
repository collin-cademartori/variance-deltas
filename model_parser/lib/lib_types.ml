(** Types defining the standard library functions
    for the model specification language.
    
    These allow basic integer computations, mostly
    to allow for more complex indexing of multidimensional
    variables when specifying which variables are involved
    in which factors of the distribution. *)

open! Sexplib.Std

open Static_env.StaticEnv

(** Function type with a constructor for each standard
    library function. *)
type func_t = Add | Max | Min | Sub
[@@deriving sexp]

(** [func_exp] maps valid function expressions to the
    corresponding standard library type. *)
let func_exp = [
  ("add", Add);
  ("max", Max);
  ("min", Min);
  ("sub", Sub);
]

(** Types of arguments and return values for each standard
    library function. *)
let func_types = [
  (Add, ([Int_T; Int_T], Int_T));
  (Max, ([Int_T; Int_T], Int_T));
  (Min, ([Int_T; Int_T], Int_T));
  (Sub, ([Int_T; Int_T], Int_T))
]