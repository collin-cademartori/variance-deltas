open Static_env.StaticEnv
open Ast

exception TypeError of string

let rec index_type = function
  | [] -> None
  | Int_T :: xs -> Some (true && Option.value (index_type xs) ~default:true)
  | IntArray_T :: _ -> Some false
  | _ :: _ -> raise (TypeError "Indices must be integer or integer arrays.")

let rec typeof_ss env = function
  | Ast.Lit _ -> Float_T
  | Ast.LitInt _ -> Int_T
  | Ast.Range (x, y) ->
    if(typeof_ss env x == Int_T && typeof_ss env y == Int_T) then
      IntArray_T
    else raise (TypeError "Range requires integer limits.")
  | Ast.Var (v, ss) -> let tv = lookup env v in
      match index_type (List.map (fun x -> typeof_ss env x) ss) with
      | None -> tv
      | Some false -> if(tv == IntArray_T || tv == FloatArray_T) then tv
        else raise (TypeError "Indices can only be applied to array types.")
      | Some true -> match tv with
        | IntArray_T -> Int_T
        | FloatArray_T -> Float_T
        | _ -> raise (TypeError "Indices can only be applied to array types.")

let rec typecheck_s env = function
  | Ast.Dist (_, sub_left, subs_right) -> 
    if (typeof_ss env sub_left == Bool_T ||
        not (List.for_all (fun x -> typeof_ss env x != Bool_T) subs_right)) 
    then
      raise (TypeError "Distribution inputs must be of numeric type.")
    else true
  | Ast.For (l_name, sub_loop, sub_inner) -> 
    if (typeof_ss env sub_loop == IntArray_T) then
      let l_env = extend env l_name Int_T in
        List.for_all (fun x -> typecheck_s l_env x) sub_inner
    else raise (TypeError "Must loop over integer array-like.")

let add_param_to_env env = function
  | Ast.Param (pn, pt, pi) -> match pi with
    | [] -> if (pt == Ast.Real) then
        extend env pn Float_T
      else raise (TypeError "Array types must be declared with at least one dimension.")
    | _::_ -> if (pt == Ast.Array) then
        extend env pn FloatArray_T
      else raise (TypeError "Dimensions can only be specified on array types.")

let check_model model =
  let model_env = List.fold_left add_param_to_env empty model.params_block in
    List.for_all (fun ms -> typecheck_s model_env ms) model.model_block