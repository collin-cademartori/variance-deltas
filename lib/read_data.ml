open Yojson.Basic.Util

module Arr = Owl.Dense.Ndarray.Generic

exception DataError of string

type int_lists =
  | I of int
  | L of int_lists array

(* let get_from_int_lists il index =
  let res_int = ref None in
  let res_il = ref il in Array.iter (fun i -> 
  begin
    match !res_il with
      | L lst -> 
      begin match lst.(i) with
        | L inner_lst -> res_il := L inner_lst
        | I inner_int -> res_int := Some inner_int
      end
      | I inner_int -> res_int := Some inner_int
  end) index;
  try Option.get !res_int with Invalid_argument _ -> raise (DataError "Index out of bounds.") *)

let rec get_from_int_lists indices = function
  | L arr -> 
  begin
    try get_from_int_lists (List.tl indices) arr.(List.hd indices)
    with Invalid_argument _ -> 
      raise (DataError ("Index out of bounds: " ^ (string_of_int (List.hd indices)) ^ ": " ^ (string_of_int ((List.length indices) - 1))))
  end
  | I i -> if (List.length indices <= 1) then i 
    else raise (DataError ("Cannot index scalar integer: " ^ (string_of_int (List.length indices))))

let to_int_singleton id = (fun i -> I i) (to_int id)

let rec to_int_lists n json = if n = 0 then 
    I (to_int json)
  else if n > 0 then L (Array.of_list (List.map (fun jl -> to_int_lists (n-1) jl) (to_list json)))
  else raise (DataError "Cannot parse integer lists of dimensions less than 1.")

let to_marray dims ints = Arr.init_nd Interpreter.int_kind dims (fun index -> get_from_int_lists (Array.to_list index) ints)

let read_from_json json d_name d_dims = let is_scalar = Array.length d_dims = 0  in
  json 
  |> member d_name
  |> (if is_scalar then to_int_singleton else to_int_lists (Array.length d_dims))
  |> to_marray (if is_scalar then [|1|] else d_dims)

let parse_datum input_json data_env = function
  | Ast.Data (d_name, _, dis) -> 
    let dims = Array.of_list (List.map (fun di -> Interpreter.expand_integer data_env di) dis) in
    let datum = read_from_json input_json d_name dims in
    (d_name, datum) :: data_env