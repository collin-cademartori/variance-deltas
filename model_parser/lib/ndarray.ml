type ('a, 'b) t = ('a, 'b, Bigarray.c_layout) Bigarray.Genarray.t

let create kind shape v =
  let arr = Bigarray.Genarray.create kind Bigarray.c_layout shape in
  Bigarray.Genarray.fill arr v; arr

let get arr index = Bigarray.Genarray.get arr index

let shape arr = Bigarray.Genarray.dims arr

let init kind shape f =
  assert (Array.length shape = 1);
  let arr = Bigarray.Genarray.create kind Bigarray.c_layout shape in
  let n = shape.(0) in
  for i = 0 to n - 1 do
    Bigarray.Genarray.set arr [|i|] (f i)
  done; arr

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

let sequential kind ~a ~step shape =
  let arr = Bigarray.Genarray.create kind Bigarray.c_layout shape in
  let n = shape.(0) in
  for i = 0 to n - 1 do
    Bigarray.Genarray.set arr [|i|] (a + i * step)
  done; arr

let to_array arr =
  let n = Bigarray.Genarray.dims arr in
  let total = Array.fold_left ( * ) 1 n in
  let flat = Bigarray.reshape_1 arr total in
  Array.init total (fun i -> Bigarray.Array1.get flat i)
