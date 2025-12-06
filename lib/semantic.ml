open Static_env.StaticEnv
open Ast

exception TypeError of string

let rec index_type = function
  | [] -> None
  | Int_T :: xs -> Some (true && Option.value (index_type xs) ~default:true)
  | (IntArray_T 1) :: _ -> Some false
  | (IntArray_T _) :: _ -> raise (TypeError "Only one-dimension integer arrays can be used as indices.")
  | _ :: _ -> raise (TypeError "Indices must be integer or integer arrays.")

let rec typeof_stmt env = function
  | Ast.Lit _ -> Float_T
  | Ast.LitInt _ -> Int_T
  | Ast.Range (x, y) ->
    if(typeof_stmt env x == Int_T && typeof_stmt env y == Int_T) then
      IntArray_T 1
    else raise (TypeError "Range requires integer limits.")
  | Ast.Var (v, ss) -> let tv = lookup env v in
      match index_type (List.map (fun x -> typeof_stmt env x) ss) with
      | None -> tv
      | Some false -> begin match tv with
        | IntArray_T dim -> if(List.length ss == dim) then tv
          else raise (TypeError "Incorrect number of dimension specified.")
        | FloatArray_T dim -> if(List.length ss == dim) then tv
          else raise (TypeError "Incorrect number of dimension specified.")
        | _ -> raise (TypeError "Indices can only be applied to array types.")
      end
      | Some true -> begin match tv with
        | IntArray_T dim -> if (List.length ss == dim) then Int_T
          else raise (TypeError "Incorrect number of dimension specified.")
        | FloatArray_T dim -> if (List.length ss == dim) then Float_T
          else raise (TypeError "Incorrect number of dimension specified.")
        | _ -> raise (TypeError "Indices can only be applied to array types.")
      end

let rec typecheck_s env = function
  | Ast.Dist (_, sub_left, subs_right) -> 
    if (typeof_stmt env sub_left == Bool_T ||
        not (List.for_all (fun x -> typeof_stmt env x != Bool_T) subs_right)) 
    then
      raise (TypeError "Distribution inputs must be of numeric type.")
    else true
  | Ast.For (l_name, sub_loop, sub_inner) -> 
    if (typeof_stmt env sub_loop == IntArray_T 1) then
      let l_env = extend env l_name Int_T in
        List.for_all (fun x -> typecheck_s l_env x) sub_inner
    else raise (TypeError "Must loop over one-dimensional integer array-like.")

let add_param_to_env env = function
  | Ast.Param (pn, pt, pi) -> match pi with
    | [] -> if (pt == Ast.Real) then
        extend env pn Float_T
      else raise (TypeError "Array types must be declared with at least one dimension.")
    | _::_ -> if (pt == Ast.Array) then
        if List.for_all (fun st -> typeof_stmt env st == Int_T) pi then
          extend env pn (FloatArray_T (List.length pi))
        else raise (TypeError "Dimensions must have integer type.")
      else raise (TypeError "Dimensions can only be specified on array types.")

let add_datum_to_env env = function

  | Ast.Data (dn, dt, di) -> match di with
    | [] -> if (dt == Ast.Int) then
        extend env dn Int_T
      else if (dt == Ast.Bool) then
        extend env dn Bool_T
      else raise (TypeError "Array types must be declared with at least one dimension.")
    | _::_ -> if (dt == Ast.IArray) then
        if List.for_all (fun st -> typeof_stmt env st == Int_T) di then
          extend env dn (IntArray_T (List.length di))
        else raise (TypeError "Dimensions must have integer type.")
      else raise (TypeError "Dimensions can only be specified on array types.")

let check_model model =
  try begin
    let data_env = List.fold_left add_datum_to_env empty model.data_block in
      let model_env = List.fold_left add_param_to_env data_env model.params_block in
        ignore (List.for_all (fun ms -> typecheck_s model_env ms) model.model_block)
  end with
    | TypeError msg -> print_endline ("Type checking failed with error: " ^ msg)
    | LookupError var_n -> print_endline ("Type checking failed due to variable not in scope: " ^ var_n)