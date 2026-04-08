(** Reads in data from JSON files and constructs a data environment mapping
    declared variable names to the corresponding parsed data.
    
    Successful parsing requires that the JSON data have keys corresponding
    to all declared variable names and that the type of the data accessed at those keys
    corresponds correctly to the type (including dimensions) declared for the
    data variable.

    Parsing strategy is two-fold:
      {- First, convert Yojson parse tree into a more constrained tree-strucutred
        data type [int_lists] while validating JSON datatypes and dimensions and converting
        integer data into the OCaml [int] type.}
      {- Second, convert from tree-structured representation to a multidimensional
        [Bigarray.Genarray] datatype which is more efficient and directly useful for
        computation later in the interpretation pipeline.}

    The double-conversion and need to walk two tree structured (the Yojson tree and the
    internal [int_lists] tree) is suboptimal, but this module is used only once at the
    beginning of the interpreter pipeline, limiting its performance impact overall. *)

open Yojson.Basic.Util

exception DataError of string * Ast.meta
exception ConversionError of string * int
exception IndexError of int * int
exception LocalError of string 


type int_lists =
  | I of int
  | L of int_lists array

let dtype_is_integer = function
  | Ast.Int -> true
  | Ast.IArray -> true
  | _ -> false

let rec get_from_int_lists indices = function
  | L arr -> 
  begin
    try get_from_int_lists (List.tl indices) arr.(List.hd indices)
    with Invalid_argument _ -> 
      raise (IndexError (List.hd indices, List.length indices))
  end
  | I i -> if (List.length indices <= 1) then i 
    else raise (LocalError ("JSON array has " ^ string_of_int (List.length indices) ^ " too few dimensions."))

let to_int_singleton id = (fun i -> I i) (to_int id)

let rec to_int_lists n json = if n = 0 then try
    I (to_int json)
    with Type_error (msg, _) -> raise (ConversionError (msg, n))
  else if n > 0 then try L (Array.of_list (List.map (fun jl -> to_int_lists (n-1) jl) (to_list json)))
  with Type_error (msg,_) -> raise (ConversionError (msg, n))
  else raise (LocalError "Cannot parse integer lists of dimensions less than 1.")

(** [to_marray dims ints] converts the int_lists [ints] into a mutlidimensional
    [Bigarray.Genarray] with dimensions [dims]. *)
let to_marray dims ints = if ints = L [||] then None else Some
 (Ndarray.init_nd Interpreter.int_kind dims (fun index -> 
   try get_from_int_lists (Array.to_list index) ints
   with IndexError (i_err, n_rem) -> raise (LocalError (
      "Error indexing dimension "
      ^ string_of_int ((Array.length index)  - n_rem + 1)
      ^ " of JSON array: " ^ string_of_int (i_err + 1) ^ " is out of bounds."
    ))
  ))

(** [to_lists n_dims json] converts JSON data [json] to a multidimensional list of
    integer values with dimensions [n_dims]. *)
let to_lists n_dims json = if (json = `Null) then (L [||]) 
  else if n_dims = 0 then 
    to_int_singleton json 
  else try to_int_lists n_dims json with
  | ConversionError (msg, n) -> 
    raise (LocalError ("At dimension " ^ string_of_int( n_dims - n) ^ ": " ^ msg))
  
(** [read_from_json json d_name d_dims] extracts the value at key [d_name] from JSON
    [json] and converts it to a array of integers with dimensions [d_dims]. Scalar
    integer data ([d_dims = [||]]) is promoted to a singleton array. *)
let read_from_json json d_name d_dims = let n_dims  = Array.length d_dims in
  json 
  |> member d_name
  |> to_lists n_dims
  |> to_marray (if n_dims = 0 then [|1|] else d_dims)

(** [parse_datum input_json data_env data_dec] converts the {!Ast.data_dec_stmt} [data_dec]
    to a pair mapping the data variable name to the corresponding integer array parsed from the
    JSON data [input_json]. The pre-existing data environment [data_env] is used to
    evaluate the dimensions of the declared data variable, which are needed to validate
    the parsing of [input_json]. *)
let parse_datum input_json data_env = function
  | Ast.Data (d_name, d_type, dis, loc) -> 
    if (dtype_is_integer d_type) then begin
      let dims = try Array.of_list (List.map (fun di -> Interpreter.expand_integer data_env di) dis) 
        with Interpreter.DataError msg -> raise (DataError (msg, loc)) in
      let datum = try read_from_json input_json d_name dims
      with 
        | LocalError msg -> raise (DataError ("Error parsing JSON for data variable " ^ d_name ^ ": " ^ msg, loc)) 
        | Type_error (msg,_) -> raise (DataError ("Error parsing JSON for data variable " ^ d_name ^ ": " ^ msg,loc))
        | _ -> raise (DataError ("Unknown error while attempting to read input data for data variable " ^ d_name ^ ".", loc))
      in (d_name, datum) :: data_env
    end else (d_name, None) :: data_env

(** [parse_data dblock json] folds over each data declaration statement in [dblock]
    and constructs an association list mapping each variable name to the corresponding
    parsed data contained in [json]. *)
let parse_data dblock json = List.fold_left (fun env ddec -> parse_datum json env ddec) [] dblock