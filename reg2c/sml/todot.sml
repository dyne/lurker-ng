structure T = Automata(Alphabet)
structure DFA = T.Deterministic
structure NFA = T.NonDeterministic
structure E = T.Expression
structure RE = T.RegularExpression

(* fun status m = (TextIO.output (TextIO.stdErr, m); TextIO.flushOut TextIO.stdErr) *)
fun status m = ()

val args = CommandLine.arguments ()
fun andexp (s, a) = E.Intersect (a, RE.toExpression (RE.fromString s))
val exp = case args of [] => E.Star E.Any
                     | (x::[]) => RE.toExpression (RE.fromString x)
                     | (x::l) => foldl andexp (RE.toExpression (RE.fromString x)) l

val (nfa, dfa, opt) = 
   case OS.Path.file (CommandLine.name ()) of
      "todot-nfa" => (true, false, false)
    | "todot-dfa" => (false, true, false)
    | "todot-opt" => (false, false, true)
    | _ => (true, true, true)

val n = E.toNFA exp
val () = status ("NFA states: " ^ Int.toString (NFA.size n) ^ "\n")
val () = if nfa then print (NFA.toDot ("nfa", n)) else ()
val d = NFA.toDFA n
val () = status ("DFA states: " ^ Int.toString (DFA.size d) ^ "\n")
val () = if dfa then print (DFA.toDot ("dfa", d)) else ()
val d = DFA.optimize d
val () = status ("DFA states: " ^ Int.toString (DFA.size d) ^ " (optimized)\n")
val () = if opt then print (DFA.toDot ("optdfa", d)) else ()
