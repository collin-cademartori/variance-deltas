(** A lightweight wrapper around [Bigarray.Genarray] with some
    simple helper functions for initialization and conversion to
    [array]. *)

type ('a, 'b) t = ('a, 'b, Bigarray.c_layout) Bigarray.Genarray.t

(** Create an [ndarray] of given [shape] with all entries set to [v]. *)
let create kind shape v =
  let arr = Bigarray.Genarray.create kind Bigarray.c_layout shape in
  Bigarray.Genarray.fill arr v; arr

(** Retrieve element of [arr] at index [index]. *)
let get arr index = Bigarray.Genarray.get arr index

(** The shape of [arr] as an [int array]. *)
let shape arr = Bigarray.Genarray.dims arr

(** Initialize one-dimensional [ndarray] with element [i] equal to [f(i)]. *)
let init kind shape f =
  assert (Array.length shape = 1);
  let arr = Bigarray.Genarray.create kind Bigarray.c_layout shape in
  let n = shape.(0) in
  for i = 0 to n - 1 do
    Bigarray.Genarray.set arr [|i|] (f i)
  done; arr

(** Initialize [ndarray] with shape [dims], where each element is initialized
    by [f] evaluated at the corresponding index (an [int array]). *)
let init_nd kind dims f =
  let arr = Bigarray.Genarray.create kind Bigarray.c_layout dims in
  let ndims = Array.length dims in
  let index = Array.make ndims 0 in
  let rec loop d =
    if d = ndims then
      Bigarray.Genarray.set arr (Array.copy index) (f (Array.copy index))
    else
      for i = 0 to dims.(d) - 1 do
        index.(d) <- i;
        loop (d + 1)
      done
  in
  loop 0; arr

(** Initialize a one-dimensional [ndarray] with elements taken from the arithmetic
    sequence beginning at [a] with step-size [step]. *)
let sequential kind ~a ~step shape =
  let arr = Bigarray.Genarray.create kind Bigarray.c_layout shape in
  let n = shape.(0) in
  for i = 0 to n - 1 do
    Bigarray.Genarray.set arr [|i|] (a + i * step)
  done; arr

(** Flatten [arr] and convert to an OCaml [array]. *)
let to_array arr =
  let n = Bigarray.Genarray.dims arr in
  let total = Array.fold_left ( * ) 1 n in
  let flat = Bigarray.reshape_1 arr total in
  Array.init total (fun i -> Bigarray.Array1.get flat i)
