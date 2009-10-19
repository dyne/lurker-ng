(* Relevant RFCs:
 *  3986 -- uri scheme (we only want to match urls tho)
 *  3696 -- url schemes
 *  1738 -- http/ftp schemes
 *  2822 -- email address format
 *  2181 -- domain name syntax
 *)

fun repl c s = String.translate (fn d => if c = d then s else String.str d)

(* RFC3986: *)
val A = "[a-zA-Z]"        (* ALPHA *)
val D = "[0-9]"           (* DIGIT *)
val H = "[0-9a-fA-F]"     (* HEXDIG *)
val U = "[a-zA-Z0-9._~-]" (* unreserved *)
val G = "[]:/?#[@]"       (* gen-delims *)
val S = "[!$&'()*+,;=]"   (* sub-delims *)
val R = "("^G^"|"^S^")"   (* reserved *)

val pct_encoded = "%"^H^H
val P = "("^U^"|"^pct_encoded^"|"^S^"|[:@])" (* pchar *)

val dec_octet = "([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])"
val ipv4 = dec_octet ^ "\\." ^ dec_octet ^ "\\." ^ dec_octet ^ "\\." ^ dec_octet

val h16  = H^"{1,4}"
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
val ipvf = "v"^H^"+\\.("^U^"|"^S^"|:)+"
val iplit = "\\[(" ^ ipv6 ^ "|" ^ ipvf ^ ")\\]"

(* RFC 1738 (3986 is too forgiving!), but allow trailing period *)
val toplabel = "[a-zA-Z](|[a-zA-Z0-9-]*[a-zA-Z0-9])"
val domainlabel = "[a-zA-Z0-9](|[a-zA-Z0-9-]*[a-zA-Z0-9])"
val hostname = "("^domainlabel^"\\.)*"^toplabel^"\\.?"
(* %HH encoding in a domain is rejected by firefox ... so we do too *)
(* val reg_name = "("^U^"|"^pct_encoded^"|"^S^")*" *)

(* allow empty since we omit it in our 'hostname' rule *)
val host = "(" ^ iplit ^ "|" ^ ipv4 ^ "|" ^ hostname ^ "|)"


(* RFC3986 and RFC1738 disagree about login format 
 * RFC3986 allows ~ and multiple :s to separate more than user:pass
 * RFC1738 allows ? and only one :
 * We allow both ~ and ? and only allow one :
 *)
(*
 val login = "([a-zA-Z0-9] [$-_.+] [!*'(),] [;?&=]"^pct_encoded^")" (* 1738 *)
 val login = "([a-zA-Z0-9] [._~-] [!$&'()*+,;=] [:]"^ pct_encoded^")" (* 3986 *)
*)
val login = "("^U^"|"^S^"|[?]|"^pct_encoded^")*"
val pass = login
val userinfo = login ^ "(:" ^ pass ^ ")?"

(* RFC3986 allows too much... *)
(* val scheme = "[a-zA-Z][a-zA-Z0-9+.-]*" *)
val scheme = "([hH][tT]{2}[pP][sS]?|[fF][tT][pP][sS]?|[fF][iI][lL][eE]|[nN][eE][wW][sS]|[iI][mM][aA][pP][sS]?)"
val authority = "(" ^ userinfo ^ "@)?" ^ host ^ "(:[0-9]*)?"

val path = P^"*"
val query = "("^P^"|"^"[/?])*"
val fragment = query

(* This is the RFC3986 definition, but it matches too much:
val hier_part = "(//" ^ authority ^ path_abempty ^ "|" ^
                        path_absolute ^ "|" ^
                        path_rootless ^ "|" ^
                        path_empty ^ ")"
*)
val hier_path = "//" ^ authority ^ "(/" ^ path ^ ")*"
val url = scheme ^ ":" ^ hier_path (* query part later *)

(* We'd like to match a bit more ... *)
val inline = "(" ^ userinfo ^ "@)?" ^
             "([wW]{3}|[fF][tT][pP])\\." ^ 
             hostname ^ "(/" ^ path ^ ")*"  (* query part later *)

(* from RFC2822:
   local-part      =       dot-atom / quoted-string / obs-local-part 
   dot-atom        =       [CFWS] dot-atom-text [CFWS]
   dot-atom-text   =       1*atext *("." 1*atext)
   quoted-string   =       [CFWS]
                           DQUOTE *([FWS] qcontent) [FWS] DQUOTE
                           [CFWS]
   atext           =       ALPHA / DIGIT / ; Any character except controls,
                           [!#$%&'*+-/=?^_`{|}~]
   ... but " is not allowed in a URL, so quoted-string doesn't matter
   ... we also ignore the CFWS as URLs don't allow this either
   these characters are a superset of U|S - (),;
   the (),; might appear in a quoted-string, though... just accept:
*)
val atext = "("^U^"|"^S^"|"^pct_encoded^")+"
val local_part = atext ^ "(\\." ^ atext ^ ")*"
val mailto = "[mM][aA][iI][lL][tT][oO]:" ^ local_part ^ "@" ^ hostname

val regexp = "(" ^ inline ^ "|" ^ mailto ^ "|" ^ url ^ ")" ^ 
             "(\\?" ^ query ^ ")?(#" ^ fragment ^ ")?"

structure T = Automata(Alphabet)
structure DFA = T.Deterministic
structure NFA = T.NonDeterministic
structure E = T.Expression
structure RE = T.RegularExpression

(* not a standard! a good heuristic to stop before punctuation *)
val fromStr = RE.toExpression o RE.fromString
val exp = E.Intersect (fromStr regexp, fromStr ".*[a-zA-Z0-9/]")
val () = print (DFA.toLongestMatchC ("find_url_end", E.toDFA exp))

val exp = E.reverse (E.Concat (exp, E.Star E.Any))
val () = print (DFA.toScannerC ("find_url_starts", E.toDFA exp))
