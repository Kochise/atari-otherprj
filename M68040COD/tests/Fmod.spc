#testing fmod	-- modulo remainder
#		-- converts the source operand to extended precision i(if
#		-- necessary) and calculates the modulo remainder of the number
#		-- in the destination floating-point data register, using the
#		-- the source operand as the modulus.  stores the result in
#		-- the destination floating-point data register, and stores 
#		-- the sign and seven least significant bits of the quotient
#		-- in the fpsr quotient byte(the quotient is the result of
#		-- fpn(/) source).  the modulo remainder function is defined as:
#		-- 	fpn -(source x n) 
#		-- where:
#		--    	n=int(fpn(/)source) in the the round-to-zero mode
#		-- the fmod function is not defined for a source operand equal
#		-- to zero or for a destination operand equal to infinity. note
#		-- that this function is not the same as the frem instruction,
#		-- which uses the round-to-nearest mode and thus returns the 
#		-- remainder that is required by the ieee specification for
#		-- binary floating-point arithmetic.


#test fmod of +1 mod +inf
fmod.d [d]3ff0_0000_0000_0000 7ff0_0000_0000_0000 rn x = 3fff00008000000000000000(1) ~N ~Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP


#test fmod of +1 mod -inf
fmod.d [d]3ff0_0000_0000_0000 fff0_0000_0000_0000 rn x = 3fff00008000000000000000(1) ~N ~Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:80 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP


#test fmod of -1 mod +inf
fmod.d [d]bff0_0000_0000_0000 7ff0_0000_0000_0000 rn x = bfff00008000000000000000(-1) N ~Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:80 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of -1 mod -inf
fmod.d [d]bff0_0000_0000_0000 fff0_0000_0000_0000 rn x = bfff00008000000000000000(-1) N ~Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of +0 mod +0
fmod.d [d]0000_0000_0000_0000 0000_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of +0 mod -0
fmod.d [d]0000_0000_0000_0000 8000_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of -0 mod +0
fmod.d [d]8000_0000_0000_0000 0000_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of -0 mod -0
fmod.d [d]8000_0000_0000_0000 8000_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of +0 mod +inf
fmod.d [d]0000_0000_0000_0000 7ff0_0000_0000_0000 rn x = 000000000000000000000000(0) ~N Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of +0 mod -inf
fmod.d [d]0000_0000_0000_0000 fff0_0000_0000_0000 rn x = 000000000000000000000000(0) ~N Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:80 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of -0 mod +inf
fmod.d [d]8000_0000_0000_0000 7ff0_0000_0000_0000 rn x = 800000000000000000000000(0) N Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:80 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of -0 mod -inf
fmod.d [d]8000_0000_0000_0000 fff0_0000_0000_0000 rn x = 800000000000000000000000(0) N Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of +inf mod +1
fmod.d [d]7ff0_0000_0000_0000 3ff0_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of +inf mod -1
fmod.d [d]7ff0_0000_0000_0000 bff0_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of -inf mod +1
fmod.d [d]fff0_0000_0000_0000 3ff0_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of -inf mod -1
fmod.d [d]fff0_0000_0000_0000 bff0_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of +inf mod +0
fmod.d [d]7ff0_0000_0000_0000 0000_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of +inf mod -0
fmod.d [d]7ff0_0000_0000_0000 8000_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of -inf mod +0
fmod.d [d]fff0_0000_0000_0000 0000_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of -inf mod -0
fmod.d [d]fff0_0000_0000_0000 8000_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of +inf mod +inf
fmod.d [d]7ff0_0000_0000_0000 7ff0_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of +inf mod -inf
fmod.d [d]7ff0_0000_0000_0000 fff0_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of -inf mod +inf
fmod.d [d]fff0_0000_0000_0000 7ff0_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of -inf mod -inf
fmod.d [d]fff0_0000_0000_0000 fff0_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP



#test fmod of +0 mod +qnan
fmod.d [d]0000_0000_0000_0000 7fff_ffff_ffff_ffff rn x = 7fff00007ffffffffffff800(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of +0 mod -qnan
fmod.d [d]0000_0000_0000_0000 ffff_ffff_ffff_ffff rn x = ffff00007ffffffffffff800(-nan0xffffffff) N ~Z ~I NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of -0 mod +qnan
fmod.d [d]8000_0000_0000_0000 7fff_ffff_ffff_ffff rn x = 7fff00007ffffffffffff800(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of -0 mod -qnan
fmod.d [d]8000_0000_0000_0000 ffff_ffff_ffff_ffff rn x = ffff00007ffffffffffff800(-nan0xffffffff) N ~Z ~I NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of +qnan mod +0
fmod.d [d]7fff_ffff_ffff_ffff 0000_0000_0000_0000 rn x = 7fff00007ffffffffffff800(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of -qnan mod +0
fmod.d [d]ffff_ffff_ffff_ffff 0000_0000_0000_0000 rn x = ffff00007ffffffffffff800(-nan0xffffffff) N ~Z ~I NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of +qnan mod -0
fmod.d [d]7fff_ffff_ffff_ffff 8000_0000_0000_0000 rn x = 7fff00007ffffffffffff800(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of -qnan mod -0
fmod.d [d]ffff_ffff_ffff_ffff 8000_0000_0000_0000 rn x = ffff00007ffffffffffff800(-nan0xffffffff) N ~Z ~I NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of +snan mod +0
fmod.d [d]7ff7_ffff_ffff_ffff 0000_0000_0000_0000 rn x snan = 7fff00007ffffffffffff800(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of +snan mod -0
fmod.d [d]7ff7_ffff_ffff_ffff 8000_0000_0000_0000 rn x snan = 7fff00007ffffffffffff800(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of -snan mod +0
fmod.d [d]fff7_ffff_ffff_ffff 0000_0000_0000_0000 rn x snan = ffff00007ffffffffffff800(-nan0xffffffff) N ~Z ~I NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of -snan mod -0
fmod.d [d]fff7_ffff_ffff_ffff 8000_0000_0000_0000 rn x snan = ffff00007ffffffffffff800(-nan0xffffffff) N ~Z ~I NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of 1  positive denorm number mod +0
fmod.d [d]0000_0000_0000_0001 0000_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of 1  negative denorm number mod -0
fmod.d [d]8000_0000_0000_0001 8000_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of 1  positive denorm number mod +inf
fmod.d [d]0000_0000_0000_0001 7ff0_0000_0000_0000 rn x = 3bcd00008000000000000000(0) ~N ~Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of 1  negative denorm number mod -inf
fmod.d [d]8000_0000_0000_0001 fff0_0000_0000_0000 rn x = bbcd00008000000000000000(0) N ~Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of 1  positive denorm number mod 1 positive denorm number
fmod.d [d]0000_0000_0000_0001 0000_0000_0000_0001 rn x = 000000000000000000000000(0) ~N Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:01 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fmod of 1  negative denorm number mod 1 negative denorm number
fmod.d [d]8000_0000_0000_0001 8000_0000_0000_0001 rn x = 800000000000000000000000(0) N Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:01 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP
