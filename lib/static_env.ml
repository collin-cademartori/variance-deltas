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

  let lookup env x =
    try List.assoc x env
    with Not_found -> raise (LookupError x)

  let extend env x ty =
      let binding = List.assoc_opt x env in
      match binding with
        | None -> (x, ty) :: env
        | Some _ -> raise (LookupError ("Cannot redefine variable " ^ x ^ " after inital definition."))
end