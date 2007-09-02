structure T = Automata(Alphabet)
structure DFA = T.Deterministic
structure NFA = T.NonDeterministic
structure E = T.Expression
structure RE = T.RegularExpression

fun status m = (TextIO.output (TextIO.stdErr, m); TextIO.flushOut TextIO.stdErr)

val args = CommandLine.arguments ()
fun andexp (s, a) = E.Intersect (a, RE.toExpression (RE.fromString s))
val exp = case args of [] => E.Star E.Any
                     | (x::[]) => RE.toExpression (RE.fromString x)
                     | (x::l) => foldl andexp (RE.toExpression (RE.fromString x)) l

val n = E.toNFA exp
val () = status ("NFA states: " ^ Int.toString (NFA.size n) ^ "\n")
val () = print (NFA.toDot ("nfa", n))
val d = NFA.toDFA n
val () = status ("DFA states: " ^ Int.toString (DFA.size d) ^ "\n")
val () = print (DFA.toDot ("dfa", d))
val d = DFA.optimize d
val () = status ("DFA states: " ^ Int.toString (DFA.size d) ^ " (optimized)\n")
val () = print (DFA.toDot ("optdfa", d))
