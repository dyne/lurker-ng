structure T = Automata(Alphabet)
structure DFA = T.Deterministic
structure NFA = T.NonDeterministic
structure E = T.Expression
structure RE = T.RegularExpression

fun repl c s = String.translate (fn d => if c = d then s else String.str d)

(* RFC3986: *)
val A = "[a-zA-Z]"        (* ALPHA *)
val D = "[0-9]"           (* DIGIT *)
val H = "[0-9a-fA-F]"     (* HEXDIG *)
val U = "[a-zA-Z0-9._~-]" (* unreserved *)
val G = "[]:/?#[@]"       (* gen-delims *)
val S = "[!$&'()*+,;=]"   (* sub-delims *)
val R = "("^G^"|"^S^")"   (* reserved *)

val dec_octet = "([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])"
val ipv4 = dec_octet ^ "\\." ^ dec_octet ^ "\\." ^ dec_octet ^ "\\." ^ dec_octet

val h16  = H^"{1,4}"
val l32 = "(H:H|" ^ ipv4 ^ ")" (* l32 *)
val ipv6 = "(" ^ "(H:){6}L|"
               ^ "::(H:){5}L|"
               ^ "H?::(H:){4}L|"
               ^ "((H:){0,1}H)?::(H:){3}L|"
               ^ "((H:){0,2}H)?::(H:){2}L|"
               ^ "((H:){0,3}H)?::H:L|"
               ^ "((H:){0,4}H)?::L|"
               ^ "((H:){0,5}H)?::H|"
               ^ "((H:){0,6}H)?::" ^ ")"
val ipv6 = repl #"H" h16 (repl #"L" l32 ipv6)
val ipvf = "v"^H^"+\\.("^U^"|"^S^"|:)+"
val iplit = "\\[(" ^ ipv6 ^ "|" ^ ipvf ^ ")\\]"
val pct_encoded = "%"^H^H
val reg_name = "("^U^"|"^pct_encoded^"|"^S^")*"
val host = "(" ^ iplit ^ "|" ^ ipv4 ^ "|" ^ reg_name ^ ")"

(*
val scheme = "[a-zA-Z][a-zA-Z0-9+\\-.]*"
val authority = "(" ^ userinfo ^ "@)?" ^ host ^ "(:[0-9]* )?"
val hier_part = "(//" ^ authority ^ path_abempty ^ "|" ^
                        path_absolute ^ "|" ^
                        path_rootless ^ "|" ^
                        path_empty ^ ")"
val uri = scheme ^ ":" ^ hier_part ^ "(\\?" ^ query ^ ")?(#" ^ fragment ^ ")?"
*)

val regexp = host
val () = print (regexp ^ "\n")

val str :: _ = CommandLine.arguments ()
val dfa = (E.toDFA o RE.toExpression o RE.fromString) regexp
val () = print (if DFA.test dfa str then "Accept\n" else "Reject\n")
