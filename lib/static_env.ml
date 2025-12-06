module StaticEnv = struct
  exception LookupError of string * string

  type ppl_type =
    | Int_T
    | Bool_T
    | Float_T
    | IntArray_T
    | FloatArray_T

  type t = (string * ppl_type) list

  let empty = []

  let lookup env x =
    try List.assoc x env
    with Not_found -> raise (LookupError ("Variable not found", x))

  let extend env x ty =
      (x, ty) :: env
end