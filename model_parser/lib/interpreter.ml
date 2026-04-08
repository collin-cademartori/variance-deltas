(** Evaluation of model ASTs. 

    Two types of evaluation are implemented:
    (i) evaluating the model block to a factor graph, represented as an
    association list mapping factor names to corresponding parameters, and (ii)
    evaluating the tree block to a tree data, consisting of a specification of the
    subsets of parameters corresponding to leaf and root nodes. *)

exception RuntimeError of string * Ast.meta
exception BoundsError of string
exception DataError of string

(** An association list mapping factor names to names of parameters involved in each factor. *)
type factor_graph = (string * (string list)) list

(** Type representing user-input integer data as multi-dimensional arrays. *)
type int_data = (int, Bigarray.int_elt) Ndarray.t
let int_kind = Bigarray.Int16_unsigned

(** Association list type mapping data variable names to user-input integer data. *)
type data_environment = (string * int_data) list

(** Association list type mapping parameter names to their shape, i.e. list of dimensions. *)
type param_data = (string * (int list)) list

(** Record type tracking the (scalar) root node and list of leaf nodes. Strings are names of individual model parameters. *)
type tree_data = {
  root: string;
  leaves: string list list
}

(** Construct an [ndarray] from an [int array]. *)
let index_from_array arr = Ndarray.init int_kind [|Array.length arr|] (fun i -> arr.(i))

(** If [grid] has length [n] and [new_dim] has length [d], construct length [n*d] list by prepending each element of [new_dim]
    to each list in [grid] and concatenating the result. The result is the flattened outer product of [grid] and [new_dim]
    using the [::] operator as the product. *)
let append_dim grid new_dim =
  List.concat (List.map (fun x -> List.map (fun y -> x :: y) grid) new_dim)

(** Takes a list of lists, with each internal list representing valid values for its
    dimension, and returns the expanded grid -- a list of lists where each internal list
    is a valid coordinate consisting of one element from each of the input lists.  *)
let rec expand_grid = function
  | ext :: [] -> List.map (fun x -> x :: []) ext
  | ext :: exts -> append_dim (expand_grid exts) ext
  | [] -> []

(** [A = get_fancy_int exts arr] is the [ndarray] with shape [[|length exts.(0); length exts.(1); ...|]] obtained
    by indexing [arr] at each combination of indices in [exts]. If [length exts = 2], then
    [A.(i).(j) = arr.(exts.(0).(i) - 1).(exts.(1).(j) - 1)]. Note that input indices in [exts] are 1-based. *)
let get_fancy_int ind_exts arr = (* let indices = expand_grid ind_exts in *)
  let dims = Array.map (fun ext -> Array.length ext) ind_exts in
  Ndarray.init_nd int_kind dims (fun index -> try Ndarray.get arr (
    Array.mapi (fun ai a -> 
      try ind_exts.(ai).(a) - 1
      with Invalid_argument _ -> begin
        Array.iter (fun ar -> print_endline (String.concat ", " (Array.to_list (Array.map (fun a -> string_of_int a) ar)))) ind_exts;
        raise (BoundsError ("Index out of bounds: " ^ (string_of_int ai) ^ ", " ^ (string_of_int a)))
      end
    ) index
  ) with Invalid_argument _ -> raise (BoundsError "Out of bounds."))

let check_index index dims = List.for_all2 (fun i d -> i <= d) index dims

(** [form_indexed_names param indices] is the corresponding list of indexed parameter expressions
    (as strings). For example [form_indexed_names "p" [[1; 2]; [2; 3]]] is 
    [["p[1,2]"; "p[1,3]"; "p[2,2]"; "p[2,3]"]] *)
let form_indexed_names vname indices param_ctx =
  let expanded_indices = expand_grid indices in
  List.map (fun index -> 
    let param_info = List.assoc vname param_ctx in
    if (check_index index param_info) then
      let index_name = List.map string_of_int index in vname ^ "[" ^ (String.concat "," index_name) ^ "]"
    else raise (BoundsError "Parameter index out of bounds.")
  ) expanded_indices

(** [can_flatten arr] is true if the shape of [arr] has at most one dimension of size > 1. *)
let can_flatten arr = 
  (Array.fold_left (fun count dim -> if(dim > 1) then count+1 else count) 0 (Ndarray.shape arr)) <= 1

(** Given a list of [ndarray] elements, flatten each and convert to list. Throws [DataError] if any
    [ndarray] elements cannot be flattened (i.e. if more than one dimension has size greater than 1). *)
let to_int_lists index_arrs = List.map
  (fun i_arr -> if (can_flatten i_arr) then Array.to_list (Ndarray.to_array i_arr)
  else raise (DataError (
    "Cannot convert multi-dimensional array to index list: " 
    ^ String.concat ", " (Array.to_list (Array.map string_of_int (Ndarray.shape i_arr)))
  ))) 
  index_arrs

let ar_singleton i = Ndarray.create int_kind [|1|] i

let ar_seq sv ev = if (sv <= ev) then Ndarray.sequential int_kind ~a:sv ~step:1 [|(ev-sv+1)|]
  else raise (DataError "Sequence start value must be less or equal to end value.")

(** [apply2_arr f s1 s2] applies the bivariate function [f] to the unique elements of the singleton arrays
    [s1] and [s2]. *)
let apply2_arr f = function
  | [a1; a2] -> if (Ndarray.shape a1 = [|1|] && Ndarray.shape a2 = [|1|]) then f (Ndarray.get a1 [|0|]) (Ndarray.get a2 [|0|]) else
      raise (DataError "Arguments not scalars.")
  | _ -> raise (DataError "Wrong number of arguments.")

(** [expand_extent data_env ast_exp] evaluates an AST expression to an [ndarray] of integers by substituting
    integer values for data variables in [data_env], evaluating library functions, expanding ranges, and indexing into
    multi-dimensional variables. Throws a [RuntimeError] when [ast_exp] cannot be evaluated, e.g. when a lower bound exceeds and
    upper bound for a range, or when an index is out of range for a data variable. *)
let rec expand_extent data_env = function
  | Ast.Lit (_, loc) -> raise (RuntimeError ("Loop extents must have integer type.", loc))
  | Ast.LitInt (i, _) -> Ndarray.create int_kind [|1|] i
  | Ast.Range (vl, vu, loc) -> let vle = expand_extent data_env vl in
    let vue = expand_extent data_env vu in
    if(Ndarray.shape vle = [|1|] && Ndarray.shape vue = [|1|]) then try
      ar_seq (Ndarray.get vle [|0|]) (Ndarray.get vue [|0|])
    with DataError msg -> raise (RuntimeError (msg, loc))
    else raise (RuntimeError ("Sequence lower and upper bounds must have integer type.", loc))
  | Ast.Func (f, arg_stmts, loc) -> 
    let args = List.map (fun ars -> expand_extent data_env ars) arg_stmts in
    begin try match f with
      | Add -> Ndarray.create int_kind [|1|] (apply2_arr (+) args)
      | Max -> Ndarray.create int_kind [|1|] (apply2_arr max args)
      | Min -> Ndarray.create int_kind [|1|] (apply2_arr min args)
      | Sub -> Ndarray.create int_kind [|1|] (apply2_arr (-) args)
    with
      | DataError err -> raise (RuntimeError ("Error applying function: " ^ err, loc))
    end
  | Ast.Var (vn, i_stmts, loc) -> 
    let indices = List.map (fun i_stmt -> expand_extent data_env i_stmt) i_stmts in
    let index_list = Array.of_list (List.map (fun ind -> Ndarray.to_array ind) indices) in try
    let v_array_opt =  List.assoc vn data_env in
      let v_array = Option.get v_array_opt in
      match index_list with
        | [||] -> v_array
        | _ -> try
          get_fancy_int index_list v_array
        with BoundsError msg -> raise (RuntimeError (msg, loc))
    with 
    | Invalid_argument _ -> raise (RuntimeError ("Data " ^ vn ^ " not found.", loc))
    | Not_found -> raise (RuntimeError ("Data " ^ vn ^ " not found.", loc))
    | Failure s -> begin
      print_endline "Tried to index array:";
      print_endline vn;
      print_endline "With indices:";
      let index_ints = try to_int_lists indices
      with DataError msg -> raise (RuntimeError (msg, loc)) in
        ignore (List.map (fun il -> print_endline (String.concat ", " (List.map (fun i -> string_of_int i) il))) index_ints);
      raise (RuntimeError ("Womp womp: " ^ s, loc))
    end

(** [expand_integer data_env stmt] is equivalent to [expand_extent data_env stmt], except that the output is assumed to be a
    singleton array, and the return value is the unique element of this array. A [DataError] is raised if the singleton
    assumption is violated. *)
let expand_integer data_env stmt =
  let maybe_int = expand_extent data_env stmt in
  if (Ndarray.shape maybe_int = [|1|]) then begin 
    try 
      Ndarray.get maybe_int [|0|]
    with
      | Not_found -> raise (DataError "Variable not found.")
    end
  else raise (DataError "Dimension size must be scalar integer.")

(** Expand an AST expression into a pair containing (1) a list of (indexed) variable names and (2) a flag indicating whether the
    expanded variable is a data variable. For example, the AST expression corresponding to "a[1:3]" with [a] a non-data variable
    is converted to [(["a[1]" "a[2]"; "a[3]"], false)]. *)
let eval_stmt param_ctx data_env = function
  | Ast.Var (vn, i_stmts, loc) -> begin match i_stmts with
    | [] -> (vn :: [], List.assoc_opt vn data_env <> None) (* failwith "Currently only support scalar values on left-hand side of sampling statements." *)
    | _ -> let indices = List.map (fun i_stmt -> expand_extent data_env i_stmt) i_stmts in
      let index_list = try to_int_lists indices
        with DataError msg -> raise (RuntimeError (msg, loc)) in
      try (form_indexed_names vn index_list param_ctx, List.assoc_opt vn data_env <> None)
      with
        | BoundsError msg -> raise (RuntimeError (msg, loc))
        | Not_found -> raise (RuntimeError ("Parameter " ^ vn ^ " not found.", loc))
    end
  | _ -> ([], false)

let get_params vs = match vs with
  | ps, false -> ps
  | _, true -> []

let get_params_or_throw vs = match vs with
  | ps, false -> ps
  | _, true -> raise (DataError "Expected parameters but got data.")

(** Evaluate an AST expression corresponding to a sample statement (individual or iterated) to an association list
    mapping factor names to corresponding lists of parameter names, appending to a the already-evaluated list [fg]. *)
let rec eval_sample fg param_ctx data_env = function
  | Ast.Dist (_, v, ps, _) -> let v_name, is_data = eval_stmt param_ctx data_env v in
    let p_names = List.concat (List.map (fun st -> get_params (eval_stmt param_ctx data_env st)) ps) in
    let d_name = if is_data then (String.concat ";" v_name) ^"_lik" else (String.concat ";" v_name) ^ "_dist" in
      (d_name, (if is_data then [] else v_name) @ p_names) :: fg
  | Ast.For (l_index, l_extent, l_body, _) -> 
    let ext_list = Array.to_list (Ndarray.to_array (expand_extent data_env l_extent)) in
    let stmt_vals = List.map (fun i_val -> 
        eval_samples_with [] param_ctx data_env l_index i_val l_body
      ) ext_list in
      (List.concat stmt_vals) @ fg
and eval_samples_with fg param_ctx data_env index index_val sstmts = 
  let new_env = ((index, Some (ar_singleton index_val)) :: data_env) in
  List.concat (List.map (fun sstmt -> eval_sample fg param_ctx new_env sstmt) sstmts)

let concat_trees tree_list = let init_tree = { root = (List.hd tree_list).root; leaves = [] } in
  List.fold_left (fun at nt -> { root = at.root; leaves = nt.leaves @ at.leaves }) init_tree tree_list

(** Evaluate an AST expression corresponding to a tree statement (root or leaf declaration, possibly iterated) to a record tracking the root
    variable's name and parameter names corresponding to each declared leaf, combining with already-evaluated tree record [tree]. *)
let rec eval_tree_stmt tree param_ctx data_env = function
  | Ast.Root (r_stmt, r_loc) -> let r_name, is_data = eval_stmt param_ctx data_env r_stmt in
    if is_data then
      raise (RuntimeError ("Tree declarations (root and leaf) cannot consist of data, only model parameters.", r_loc))
    else if List.length r_name > 1 then
      raise (RuntimeError ("Root node must be given as a single, scalar parameter.", r_loc))
    else
      { root = (List.hd r_name) ; leaves = tree.leaves }
  | Ast.Leaf (l_stmts, l_loc) -> begin try
    let l_names = List.concat (List.map (fun tt -> get_params_or_throw (eval_stmt param_ctx data_env tt)) l_stmts) in
      { root = tree.root ; leaves = l_names :: tree.leaves }
    with
      | DataError _ -> raise (RuntimeError ("All arguments to leaf declaration must be model parameters, not input data.", l_loc))
    end
  | Ast.TreeFor (l_index, l_extent, l_body, _) -> 
    let ext_list = Array.to_list (Ndarray.to_array (expand_extent data_env l_extent)) in
    let stmt_vals = List.map (fun i_val -> 
        eval_tree_stmts_with {root = tree.root; leaves = []} param_ctx data_env l_index i_val l_body
      ) ext_list in
      (concat_trees (tree :: stmt_vals))
and eval_tree_stmts_with tree param_ctx data_env index index_val tstmts = 
  let new_env = ((index, Some (ar_singleton index_val)) :: data_env) in
  concat_trees (List.map (fun tstmt -> eval_tree_stmt tree param_ctx new_env tstmt) tstmts)

let eval_param param_ctx data_env = function
  | Ast.Param (pname, _, pis, loc) -> 
    let dims = try List.map (fun pi -> expand_integer data_env pi) pis
    with DataError msg -> raise (RuntimeError (msg, loc)) in
    (pname, dims) :: param_ctx

let eval_params data_env param_block =
  List.fold_left (fun pd param -> eval_param pd data_env param) [] param_block

let eval_datum param_ctx data_env = function
  | Ast.Data (dname, _, dis, loc) -> 
    let dims = try List.map (fun di -> expand_integer data_env di) dis
    with DataError msg -> raise (RuntimeError (msg, loc)) in
    (dname, dims) :: param_ctx

let eval_data data_env data_block =
  List.fold_left (fun pd datum -> eval_datum pd data_env datum) [] data_block

let eval_model data_env dblock pblock mblock = 
  try begin
    let param_only_ctx = eval_params data_env pblock in
    let param_ctx = param_only_ctx @ (eval_data data_env dblock) in
    List.fold_left (fun fg ss -> eval_sample fg param_ctx data_env ss) [] mblock
  end with RuntimeError (emsg, eloc) -> 
    raise (RuntimeError ("Runtime error. " ^ emsg, eloc))

let eval_tree data_env pblock tblock =
  try begin
    let param_only_ctx = eval_params data_env pblock in
    let itree = { root = ""; leaves = [] } in
    List.fold_left (fun tree ts -> eval_tree_stmt tree param_only_ctx data_env ts) itree tblock
  end with RuntimeError (emsg, eloc) -> 
    raise (RuntimeError ("Runtime error. " ^ emsg, eloc))

