(* open! Ppx_yojson_conv_lib.Yojson_conv.Primitives *)

module Arr = Owl.Dense.Ndarray.Generic

exception RuntimeError of string

type factor_graph = (string * (string list)) list

type int_data = (int, Bigarray.int_elt) Arr.t
let int_kind = Bigarray.Int16_unsigned

type data_environment = (string * int_data) list

type param_data = (string * (int list)) list

let index_from_array arr = Arr.init int_kind [|Array.length arr|] (fun i -> arr.(i))

(* let decr_list lst = List.map (fun i -> i - 1) lst *)

let append_dim grid new_dim =
  List.concat (List.map (fun x -> List.map (fun y -> x :: y) grid) new_dim)

let rec expand_grid = function
  | ext :: [] -> List.map (fun x -> x :: []) ext
  | ext :: exts -> append_dim (expand_grid exts) ext
  | [] -> []

let get_fancy_int ind_exts arr = (* let indices = expand_grid ind_exts in *)
  let dims = Array.map (fun ext -> Array.length ext) ind_exts in
  Arr.init_nd int_kind dims (fun index -> try Arr.get arr (
    Array.mapi (fun ai a -> 
      try ind_exts.(ai).(a) - 1
      with Invalid_argument _ -> begin
        Array.iter (fun ar -> print_endline (String.concat ", " (Array.to_list (Array.map (fun a -> string_of_int a) ar)))) ind_exts;
        raise (RuntimeError ("Index out of bounds: " ^ (string_of_int ai) ^ ", " ^ (string_of_int a)))
      end
    ) index
  ) with Invalid_argument _ -> raise (RuntimeError "Out of bounds."))

let check_index index dims = List.for_all2 (fun i d -> i <= d) index dims

let form_indexed_names vname indices param_ctx =
  let expanded_indices = expand_grid indices in
  List.map (fun index -> 
    if (check_index index (List.assoc vname param_ctx)) then
      let index_name = List.map string_of_int index in vname ^ "[" ^ (String.concat "," index_name) ^ "]"
    else raise (RuntimeError "Parameter index out of bounds.")
  ) expanded_indices

let can_flatten arr = 
  (Array.fold_left (fun count dim -> if(dim > 1) then count+1 else count) 0 (Arr.shape arr)) <= 1

let to_int_lists index_arrs = List.map
  (fun i_arr -> if (can_flatten i_arr) then Array.to_list (Arr.to_array i_arr)
  else raise (RuntimeError (
    "Cannot convert multi-dimensional array to index list: " 
    ^ String.concat ", " (Array.to_list (Array.map string_of_int (Arr.shape i_arr)))
    (* ^ (string_of_int (Arr.num_dims i_arr)) *)
  ))) 
  index_arrs

let to_index_list index_arrs = List.map (fun int_l -> Owl_types_common.L int_l) (to_int_lists index_arrs)

let ar_singleton i = Arr.create int_kind [|1|] i

let ar_seq sv ev = if (sv <= ev) then Arr.sequential int_kind ~a:sv ~step:1 [|(ev-sv+1)|]
else raise (RuntimeError "Sequence start value must be less or equal to end value.")

let rec expand_extent data_env = function
  | Ast.Lit _ -> raise (RuntimeError "Loop extents must have integer type.")
  | Ast.LitInt i -> Arr.create int_kind [|1|] i
  | Ast.Range (vl, vu) -> let vle = expand_extent data_env vl in
    let vue = expand_extent data_env vu in
    if(Arr.shape vle = [|1|] && Arr.shape vue = [|1|]) then
      ar_seq (Arr.get vle [|0|]) (Arr.get vue [|0|])
    else raise (RuntimeError "Sequence lower and upper bounds must have integer type.")
  | Ast.Var (vn, i_stmts) -> 
    let indices = List.map (fun i_stmt -> expand_extent data_env i_stmt) i_stmts in
    let index_list = Array.of_list (List.map (fun ind -> Arr.to_array ind) indices) in
    let v_array = List.assoc vn data_env in try
      match index_list with
        | [||] -> v_array
        | _ -> get_fancy_int index_list v_array
    with Failure s -> begin
      print_endline "Tried to index array:";
      print_endline vn;
      print_endline "With indices:";
      let index_ints = to_int_lists indices in
        ignore (List.map (fun il -> print_endline (String.concat ", " (List.map (fun i -> string_of_int i) il))) index_ints);
      Arr.print v_array;
      raise (RuntimeError ("Womp womp: " ^ s))
    end

let expand_integer data_env stmt =
  let maybe_int = expand_extent data_env stmt in
  if (Arr.shape maybe_int = [|1|]) then Arr.get maybe_int [|0|] 
  else raise (RuntimeError "Dimension size must be scalar integer.")
    
    
let eval_stmt param_ctx data_env = function
  | Ast.Var (vn, i_stmts) -> begin match i_stmts with
    | [] -> vn :: [] (* This is wrong, need to add all indices for vn *)
    | _::_ -> 
      let indices = List.map (fun i_stmt -> expand_extent data_env i_stmt) i_stmts in
      let index_list = to_int_lists indices in
      form_indexed_names vn index_list param_ctx
    end
  | _ -> []

let rec eval_sample fg param_ctx data_env = function
  | Ast.Dist (dn, v, ps) -> let v_name = eval_stmt param_ctx data_env v in
    let p_names = List.concat (List.map (fun st -> eval_stmt param_ctx data_env st) ps) in
      (dn, v_name @ p_names) :: fg
  | Ast.For (l_index, l_extent, l_body) -> 
    let ext_list = Array.to_list (Arr.to_array (expand_extent data_env l_extent)) in
    let stmt_vals = List.map (fun i_val -> eval_samples_with fg param_ctx data_env l_index i_val l_body) ext_list in
      (List.concat stmt_vals) @ fg
and eval_samples_with fg param_ctx data_env index index_val sstmts = 
  let new_env = ((index, ar_singleton index_val) :: data_env) in
  List.concat (List.map (fun sstmt -> eval_sample fg param_ctx new_env sstmt) sstmts)

let eval_param param_ctx data_env = function
  | Ast.Param (pname, _, pis) -> 
    let dims = List.map (fun pi -> expand_integer data_env pi) pis in
    (pname, dims) :: param_ctx

let eval_params data_env param_block =
  List.fold_left (fun pd param -> eval_param pd data_env param) [] param_block

let eval_model data_env pblock mblock = 
  let param_ctx = eval_params data_env pblock in
  List.fold_left (fun fg ss -> eval_sample fg param_ctx data_env ss) [] mblock

