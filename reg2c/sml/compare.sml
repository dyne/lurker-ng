fun goodness x =
  if x = 0 then 100 else (* avoid null at nearly all costs *)
  if x <= 31 then 25 else (* avoid control characters *)
  if x = 32 then 3 else (* better than punctuation, worse than numbers *)
  if x >=  33 andalso x <=  47 then 4 else (* punctuation *)
  if x >=  48 andalso x <=  57 then 2 else (* numbers *)
  if x >=  58 andalso x <=  64 then 4 else (* punctuation *)
  if x >=  65 andalso x <=  90 then 1 else (* capital letters *)
  if x >=  91 andalso x <=  96 then 4 else (* punctuation *)
  if x >=  97 andalso x <= 122 then 1 else (* small letters *)
  if x >= 123 andalso x <= 126 then 4 else (* punctuation *)
  12 (* high ascii values, not quite as bad as control chars *)

val edgeLength = goodness o Char.ord

structure A = Automata(Alphabet)
structure RE = A.RegularExpression
structure E = A.Expression
structure DFA = A.Deterministic

fun examine (a, b) =
  let
    val convert = E.toDFA o RE.toExpression o RE.fromString
    val find = DFA.shortestMatch edgeLength
    val join = find o DFA.optimize o DFA.intersect
    val (pa, pb) = (convert a, convert b)
    val (na, nb) = (DFA.complement pa, DFA.complement pb)
    val (pas, nas, pbs, nbs) = (find pa, find na, find pb, find nb)
    val (papbs, panbs, napbs, nanbs) = 
          (join (pa, pb), join (pa, nb), join (na, pb), join (na, nb))
    
    fun length (SOME x) = 4 + String.size x
      | length NONE = 3
    fun max (x, y) = if x < y then y else x
    fun biggest (x, y, z) =  max (length x, max (length y, length z))
    val col1 = biggest(NONE, pas, nas)
    val col2 = biggest(pbs, papbs, napbs)
    val col3 = max(biggest(nbs, panbs, nanbs), 8)
    
    fun whitespace 0 = ""
      | whitespace i = " " ^ whitespace (i-1)
    fun dashes 0 = ""
      | dashes i = "-" ^ dashes (i-1)
    fun format (s, w) = 
      let val pad = w - String.size s in
      whitespace (pad div 2) ^ s ^ whitespace ((pad+1) div 2) end
    fun entry (SOME x, w) = format ("\"" ^ x ^ "\"", w)
      |	entry (NONE, w) = format ("-", w)
    
    val setrelation = case (papbs, panbs, napbs, nanbs) of
        (_, NONE, NONE, _) => "A is identical to B"
      | (NONE, _, _, NONE) => "A is the complement of B"
      | (_, NONE, _, _) => "A is a subset of B"
      | (_, _, NONE, _) => "B is a superset of A"
      | (NONE, _, _, _) => "A is disjoint from B"
      | _ => "A overlaps B"
  in
    print ("Expression A (" ^ Int.toString (DFA.size pa) ^ " states) = \"" ^ a ^ "\"\n");
    print ("Expression B (" ^ Int.toString (DFA.size pb) ^ " states) = \"" ^ b ^ "\"\n");
    print "\n";
    print ("       |" ^ whitespace col1  ^ "|" ^ format("B", col2)  ^ "|" ^ format("not(B)", col3) ^ "\n");
    print ("--------" ^ dashes col1      ^ "-" ^ dashes col2        ^ "-" ^ dashes col3        ^ "\n");
    print ("       |" ^ whitespace col1  ^ "|" ^ entry(pbs, col2)   ^ "|" ^ entry(nbs, col3)   ^ "\n");
    print ("A      |" ^ entry(pas, col1) ^ "|" ^ entry(papbs, col2) ^ "|" ^ entry(panbs, col3) ^ "\n");
    print ("not(A) |" ^ entry(nas, col1) ^ "|" ^ entry(napbs, col2) ^ "|" ^ entry(nanbs, col3) ^ "\n");
    print "\n";
    print ("Set relationship: " ^ setrelation ^ ".\n")
  end

val ()  = case CommandLine.arguments () of
    (a :: b :: []) => examine (a, b)
  | _ => print "Expect two regular expressions for arguments\n"
