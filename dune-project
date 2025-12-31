(lang dune 3.20)

(name parser-learning)

(generate_opam_files true)

(using menhir 3.0)

(source
 (github username/reponame))

(authors "Collin Cademartori <collin-cademartori@gmail.com>")

(maintainers "Collin Cademartori <collin-cademartori@gmail.com>")

(license LICENSE)

(documentation https://url/to/documentation)

(package
 (name parser-learning)
 (synopsis "A playground for learning about parser implementation in OCaml")
 (description "A")
 (depends ocaml)
 (tags
  ("add topics" "to describe" your project)))

; See the complete stanza docs at https://dune.readthedocs.io/en/stable/reference/dune-project/index.html
