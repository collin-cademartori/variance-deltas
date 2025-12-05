module StaticEnv = struct
  type ppl_type =
    | Int_T
    | Bool_T
    | Float_T
    | IntArray_T of int
    | FloatArray_T of int
    | Index_T of int

  type t = (string * ppl_type) 

  let empty = []

  let lookup env x =
    try List.assoc x env
    with Not_found -> failwith "Unbound variable"

  let extend env x ty =
      (x, ty) :: env
end