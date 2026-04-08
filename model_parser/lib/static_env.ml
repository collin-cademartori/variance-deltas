(** Defines an environment for tracking type information
    associated with variables.
    
    Implemented as an association list mapping string names
    to valid datatypes (including dimensions for array types.) *)

module StaticEnv = struct
  exception LookupError of string

  type ppl_type =
    | Int_T
    | Bool_T
    | Float_T
    | IntArray_T of int
    | FloatArray_T of int

  type t = (string * ppl_type) list

  let empty = []

  (** [lookup env x] is the type associated to name [x] in [env]. *)
  let lookup env x =
    try List.assoc x env
    with Not_found -> raise (LookupError x)

  (** [extend env x ty] is the environment containing all bindings in [env]
      and the new binding [(x, ty)]. If [x] is already bound in [env], raises
      a [LookupError]. *)
  let extend env x ty =
      let binding = List.assoc_opt x env in
      match binding with
        | None -> (x, ty) :: env
        | Some _ -> raise (LookupError ("Cannot redefine variable " ^ x ^ " after inital definition."))
end