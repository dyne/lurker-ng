(* A quoted paragraph consists of lines starting with "> " *)
val regexp = "(\n([a-zA-Z]{0,4}[>|]+ |>)[^\n]*)+"

structure T = Automata(Alphabet)
structure DFA = T.Deterministic
structure NFA = T.NonDeterministic
structure E = T.Expression
structure RE = T.RegularExpression

val exp = (RE.toExpression o RE.fromString) regexp
val () = print (DFA.toLongestMatchC ("find_quote_end", E.toDFA exp))

val exp = E.reverse (E.Concat (exp, E.Star E.Any))
val () = print (DFA.toScannerC ("find_quote_starts", E.toDFA exp))
