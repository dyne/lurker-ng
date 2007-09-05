(* Relevant RFCs:
 *  2822 -- email address format
 *  2181 -- domain name syntax
 *)

fun repl c s = String.translate (fn d => if c = d then s else String.str d)

val dec_octet = "([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])"
val ipv4 = dec_octet ^ "\\." ^ dec_octet ^ "\\." ^ dec_octet ^ "\\." ^ dec_octet

val h16  = "[0-9a-fA-F]{1,4}"
val l32 = "(H:H|" ^ ipv4 ^ ")" (* l32 *)
val ipv6 = "(" ^ "(H:){6}L|"
               ^ "::(H:){5}L|"
               ^ "(H)?::(H:){4}L|"
               ^ "((H:){0,1}H)?::(H:){3}L|"
               ^ "((H:){0,2}H)?::(H:){2}L|"
               ^ "((H:){0,3}H)?::H:L|"
               ^ "((H:){0,4}H)?::L|"
               ^ "((H:){0,5}H)?::H|"
               ^ "((H:){0,6}H)?::" ^ ")"
val ipv6 = repl #"H" h16 (repl #"L" l32 ipv6)
val ipvf = "v[0-9a-fA-F]+\\.[a-zA-Z0-9._~!$&'()*+,;:=-]+"

val toplabel = "[a-zA-Z](|[a-zA-Z0-9-]*[a-zA-Z0-9])"
val domainlabel = "[a-zA-Z0-9](|[a-zA-Z0-9-]*[a-zA-Z0-9])"
val hostname = "("^domainlabel^"\\.)+"^toplabel

val domain_lit = "\\[(" ^ ipv4 ^ "|" ^ ipv6 ^ "|" ^ ipvf ^ ")\\]"

val FWS = "([ \t]+\r\n)?[ \t]"
val atext = "[a-zA-Z0-9!#$%&'*+/=?^_`{|}~-]"
val dot_atom = atext ^ "+(\\." ^ atext ^ "+)*"
val quoted_str = "\"(("^FWS^")?([^\000\t\n\r \\\"]|\\[^\000\n\r]))*("^FWS^")?\""

(* We forbid "obsolete" email addresses *)
val local_part = "(" ^ dot_atom ^ "|" ^ quoted_str ^ ")"
val domain = "(" ^ hostname ^ "|" ^ domain_lit ^ ")"
val regexp = local_part ^ "@" ^ domain

structure T = Automata(Alphabet)
structure DFA = T.Deterministic
structure NFA = T.NonDeterministic
structure E = T.Expression
structure RE = T.RegularExpression

val () = print "#include <stdlib.h>\n\n"

val exp = (RE.toExpression o RE.fromString) regexp
val () = print (DFA.toLongestMatchC ("find_email_end", E.toDFA exp))

val exp = E.reverse (E.Concat (exp, E.Star E.Any))
val () = print (DFA.toScannerC ("find_email_starts", E.toDFA exp))
