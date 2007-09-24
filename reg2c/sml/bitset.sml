signature BITSET =
sig
  type array
  
  val tabulate: int * (int -> bool) -> array
  val sub: array * int -> bool
  val update: array * int * bool -> unit
  val modifyi: (int * bool -> bool) -> array -> unit
end

structure BitSet :> BITSET =
struct

   structure A = Word32Array
   structure W = Word32
   open A

   infix << >> &
   infix sub
   
   val word  = Word.fromInt 
   val int   = Word.toInt
   val op &  = Word.andb
   val op >> = Word.>>
   val op << = W.<<
   
   type array = int * array
   
   fun sub ((_, a), i) = 
   let val byte = int((word i) >> 0w5)
       val mask = W.<<(0w1, (word i) & 0w31)
   in  W.andb(A.sub(a, byte), mask) <> 0wx0 end
   
   fun update ((_, a), i, v) =
   let val byte = int((word i) >> 0w5)
       val mask = W.<<(0w1, (word i) & 0w31)
       val word = A.sub(a,byte)
   in  if v
       then A.update(a, byte, W.orb(word, mask))
       else A.update(a, byte, W.andb(word, W.notb mask))
   end
   
   fun modifyi f (n, a) =
   let val len = n div 32
       fun modifybyte (i, old) =
       let val i32 = i * 32
           val eob = if i < len then 0w32 else word (n - i32)
           fun modifybit (j, a) = 
               if j = eob then a else
               let val mask = W.<<(0w1, j)
                   val old = W.andb (old, mask) <> 0w0
                   val new = f (i32 + int j, old)
               in  modifybit (j + 0w1, if new then mask + a else a) end
       in  modifybit (0w0, 0w0) end
   in A.modifyi modifybyte a end

   fun tabulate (n, f) = 
     let val len = (n+31) div 32
         val a = (n, array (len, 0w0))
         val () = modifyi (fn (i, _) => f i) a
     in
       a
     end
end
