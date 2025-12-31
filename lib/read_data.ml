open Yojson.Basic.Util

module Arr = Owl.Dense.Ndarray.Generic

exception DataError of string * Ast.meta
exception ConversionError of string * int
exception IndexError of int * int
exception LocalError of string 

type int_lists =
  | I of int
  | L of int_lists array

let rec get_from_int_lists indices = function
  | L arr -> 
  begin
    try get_from_int_lists (List.tl indices) arr.(List.hd indices)
    with Invalid_argument _ -> 
      raise (IndexError (List.hd indices, List.length indices))
  end
  | I i -> if (List.length indices <= 1) then i 
    (* else raise (ConversionError ("Cannot index scalar integer: " ^ (string_of_int (List.length indices)))) *)
    else raise (LocalError ("JSON array has " ^ string_of_int (List.length indices) ^ " too few dimensions."))

let to_int_singleton id = (fun i -> I i) (to_int id)

(* (ConversionError ("At dimenson " ^ string_of_int n ^ ": " ^ msg)) *)
let rec to_int_lists n json = if n = 0 then try
    I (to_int json)
    with Type_error (msg, _) -> raise (ConversionError (msg, n))
  else if n > 0 then try L (Array.of_list (List.map (fun jl -> to_int_lists (n-1) jl) (to_list json)))
  with Type_error (msg,_) -> raise (ConversionError (msg, n))
  else raise (LocalError "Cannot parse integer lists of dimensions less than 1.")

let to_marray dims ints = if ints = L [||] then None else Some
 (Arr.init_nd Interpreter.int_kind dims (fun index -> 
   try get_from_int_lists (Array.to_list index) ints
   with IndexError (i_err, n_rem) -> raise (LocalError (
      "Error indexing dimension "
      ^ string_of_int ((Array.length index)  - n_rem + 1)
      ^ " of JSON array: " ^ string_of_int (i_err + 1) ^ " is out of bounds."
    ))
  ))

let to_lists n_dims json = if (json = `Null) then (L [||]) 
  else if n_dims = 0 then 
    to_int_singleton json 
  else try to_int_lists n_dims json with
  | ConversionError (msg, n) -> 
    raise (LocalError ("At dimenson " ^ string_of_int( n_dims - n) ^ ": " ^ msg))
  

let read_from_json json d_name d_dims = let n_dims  = Array.length d_dims in
  json 
  |> member d_name
  |> to_lists n_dims
  |> to_marray (if n_dims = 0 then [|1|] else d_dims)

let parse_datum input_json data_env = function
  | Ast.Data (d_name, d_type, dis, loc) -> 
    if (Ast.dtype_is_integer d_type) then begin
      let dims = try Array.of_list (List.map (fun di -> Interpreter.expand_integer data_env di) dis) 
        with Interpreter.DataError msg -> raise (DataError (msg, loc)) in
      let datum = try read_from_json input_json d_name dims
      with 
        | LocalError msg -> raise (DataError ("Error parsing JSON for data variable " ^ d_name ^ ": " ^ msg, loc)) 
        | Type_error (msg,_) -> raise (DataError ("Error parsing JSON for data variable " ^ d_name ^ ": " ^ msg,loc))
        | _ -> raise (DataError ("Unknown error while attempting to read input data for data variable " ^ d_name ^ ".", loc))
      in (d_name, datum) :: data_env
    end else (d_name, None) :: data_env

let parse_data dblock json = List.fold_left (fun env ddec -> parse_datum json env ddec) [] dblock