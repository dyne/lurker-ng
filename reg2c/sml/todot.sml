structure T = Automata(Alphabet)
structure DFA = T.Deterministic
structure NFA = T.NonDeterministic
structure E = T.Expression
structure RE = T.RegularExpression

val exp = (RE.toExpression o RE.fromString o hd o CommandLine.arguments) ()
val n = E.toNFA exp
val () = print (NFA.toDot ("nfa", n))
val d = NFA.toDFA n
(* val () = print ("DFA states: " ^ Int.toString (DFA.size d) ^ "\n") *)
val d = DFA.optimize d
val () = print (DFA.toDot ("dfa", d))
