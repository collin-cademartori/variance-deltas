(* open Static_env.StaticEnv *)

exception TypeError of string

(* let typeof_arg env = function
  | Ast.LitInt _ -> Int_T
  | Ast.VarInt (x, []) ->  
    if (lookup env x == Int_T) then
      Int_T 
    else raise (TypeError "Expected integer variable")
  | Ast.VarInt (x, index) -> 
    match typeof_index index with
    | Index_T dim -> Int_T
    | _ -> raise (TypeError "Invalid index type.") *)
