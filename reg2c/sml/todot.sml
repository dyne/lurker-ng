structure T = Automata(Alphabet)
structure DFA = T.Deterministic
structure NFA = T.NonDeterministic
structure E = T.Expression
structure RE = T.RegularExpression

fun status m = (TextIO.output (TextIO.stdErr, m); TextIO.flushOut TextIO.stdErr)

val exp = (RE.toExpression o RE.fromString o hd o CommandLine.arguments) ()
val n = E.toNFA exp
val () = status ("NFA states: " ^ Int.toString (NFA.size n) ^ "\n")
val () = print (NFA.toDot ("nfa", n))
val d = NFA.toDFA n
val () = status ("DFA states: " ^ Int.toString (DFA.size d) ^ "\n")
val () = print (DFA.toDot ("dfa", d))
val d = DFA.optimize d
val () = status ("DFA states: " ^ Int.toString (DFA.size d) ^ " (optimized)\n")
val () = print (DFA.toDot ("optdfa", d))
