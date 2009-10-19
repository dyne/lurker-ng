(* This regular expression looks for ascii art.
 * We identify ascii art as a paragraph containing a hard tab or four spaces.
 * Alternately, any paragraph that looks like a signature block
 *)

val nonEmptyLine = "\n[^\n]*[^ \t\n][^\n]*" (* A content character *)
val artLine1 = "\n[^\n]*[^ \t\n][^\n]*( {4}|\t)[^\n]*"
val artLine2 = "\n[^\n]*( {4}|\t)[^\n]*[^ \t\n][^\n]*"

val regexp = 
   "(" ^ nonEmptyLine ^ ")*" ^
   "(" ^ artLine1 ^ "|" ^ artLine2 ^ ")" ^
   "(" ^ nonEmptyLine ^ ")*"
   
structure T = Automata(Alphabet)
structure DFA = T.Deterministic
structure NFA = T.NonDeterministic
structure E = T.Expression
structure RE = T.RegularExpression

val exp = (RE.toExpression o RE.fromString) regexp
val () = print (DFA.toLongestMatchC ("find_art_end", E.toDFA exp))

val exp = E.reverse (E.Concat (exp, E.Star E.Any))
val () = print (DFA.toScannerC ("find_art_starts", E.toDFA exp))
