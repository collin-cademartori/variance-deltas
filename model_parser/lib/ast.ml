(** The abstract syntax tree.

    Declares all data types needed to construct
    the abstract syntax tree for a model specification.
    The top-level type is [model], which represents a
    specification as lists of statements divided into
    separate semantic blocks. Each semantic block has its
    own sub-structure defined by the corresponding types.

    The type parameter ['m] attached to each AST type
    is used to track meta data (represented by the [meta] type)
    which enables, e.g. descriptive error messages with
    line/column numbers when we encounter parsing errors.
    
    Sexp converters are derived for each type for pretty
    printing of the AST. *)

open! Sexplib.Std
open Lexing
open Lib_types

(** Lexical metadata associated to each AST node *)
type meta = position * position

(** Explicit sexp converter for [meta], used by the sexp generator *)
let sexp_of_meta (st, en) = Sexplib0.Sexp.List [
  Sexplib0.Sexp.Atom (string_of_int st.pos_cnum);
  Sexplib0.Sexp.Atom (string_of_int en.pos_cnum)
]

(** Variable data types in model specification language *)
type datatype = Bool | Int | IArray | Real | Array
[@@deriving sexp]

(** Basic semantic units: literals, variables, ranges, and function applications.
    Represents expressions that appear within block-specific statements.
    
    [Var (name, extents, meta)] represents a variable reference (not declaration),
    with extents providing optional subscripts.
    
    [Func (fname, args, meta)] represents the function application [fname(args)], where
    fname has type {!Lib_types.func_t}, which represents a standard library function. *)
type 'm stmt =
  | Lit of float * 'm
  | LitInt of int * 'm
  | Var of string * ('m stmt) list * 'm
  | Range of ('m stmt) * ('m stmt) * 'm
  | Func of func_t * ('m stmt) list * 'm
[@@deriving sexp]

(** Statement(s) assigning a distribution to a variable, where the
    distribution may also depend on other variables.
    
    [Dist (dname, var, deps, meta)] represents a single sampling
    statement [var ~ dname(deps)]. The distribution name [dname]
    is a user-defined tag with no semantic meaning.
    
    [For (index, range_stmt, sample_stmts, meta)] represents iteratively
    declared sampling statements [for(index in range_stmt) { sample_stmts }]. *)
type 'm sample_stmt = 
  | Dist of string * ('m stmt) * ('m stmt) list * 'm
  | For of string * ('m stmt) * ('m sample_stmt) list * 'm
  [@@deriving sexp]

(** Declaration of parameter name and data type, with dimension information *)
type 'm param_dec_stmt = Param of string * datatype * ('m stmt) list * 'm
[@@deriving sexp]

(** Declaration of data variable name and data type, with dimension information *)
type 'm data_dec_stmt = Data of string * datatype * ('m stmt) list * 'm
[@@deriving sexp]

(** Declaration of root and leaf nodes for generating a variance delta.
    
    [Leaf (stmts, meta)] represents a collection of parameters defining
    a leaf node in the variance delta tree.

    [TreeFor (index, range_stmt, tree_stmts)] represents iteratively declared
    leaf/root nodes [for(index in range_stmt) { tree_stmts }].
    
    [Root (stmt, meta)] represents a scalar parameter defining the root
    node of the variance delta tree. *)
type 'm tree_stmt =
  | Leaf of ('m stmt) list * 'm
  | Root of ('m stmt) * 'm
  | TreeFor of string * ('m stmt) * ('m tree_stmt) list * 'm
[@@deriving sexp]

(** Top-level type for model specification, dividing the spec into
    semantic blocks which are parsed separately. The [data_block] 
    declares variables which will have externally fixed values; the
    [params_block] represents unknown model parameters over which the spec
    declares a probability distribution; [model_block] specificies individual
    factors of the joint distribution, and [tree_block] defines subsets of
    parameters to represent root and leaf nodes in a variance delta tree. *)
type 'm model = {
  params_block : ('m param_dec_stmt) list;
  model_block : ('m sample_stmt) list;
  data_block : ('m data_dec_stmt) list;
  tree_block : ('m tree_stmt) list;
}
[@@deriving sexp]