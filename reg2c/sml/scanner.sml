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

val d = E.toDFA exp
val () = print (DFA.toLongestMatchC ("longfn", d))

val () = if DFA.accepts d (DFA.start d)
         then status "Warning: Regexp accepts the empty string; progressive scanning will hang.\n"
         else ()

val () = print "#include <stdlib.h>\n\n"

val star = E.Concat (exp, E.Star E.Any)
val dstar = E.toDFA star
val () = print (DFA.toShortestMatchC ("shortfn", dstar))

val rstar = E.reverse star
val drstar = E.toDFA rstar
val () = print (DFA.toScannerC ("scanner", drstar))
