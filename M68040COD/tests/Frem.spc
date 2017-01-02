#testing frem	-- ieee remainder
#		-- converts the source operand to extended precision i(if
#		-- necessary) and calculates the remulo remainder of the number
#		-- in the destination floating-point data register, using the
#		-- the source operand as the remulus.  stores the result in
#		-- the destination floating-point data register, and stores 
#		-- the sign and seven least significant bits of the quotient
#		-- in the fpsr quotient byte(the quotient is the result of
#		-- fpn(/) source).  the ieee remainder function is defined as:
#		-- 	fpn -(source x n) 
#		-- where:
#		--    	n=int(fpn(/)source) in the the round-to-nearest reme
#		-- the frem function is not defined for a source operand equal
#		-- to zero or for a destination operand equal to infinity. note
#		-- that this function is not the same as the frem instruction,
#		-- which uses the round-to-zero reme and thus returns the 
#		-- remainder that is different from the ieee specification for
#		-- binary floating-point arithmetic.


#test frem of +1 rem +inf
frem.d [d]3ff0_0000_0000_0000 7ff0_0000_0000_0000 rn x = 3fff00008000000000000000(1) ~N ~Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP


#test frem of +1 rem -inf
frem.d [d]3ff0_0000_0000_0000 fff0_0000_0000_0000 rn x = 3fff00008000000000000000(1) ~N ~Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:80 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP


#test frem of -1 rem +inf
frem.d [d]bff0_0000_0000_0000 7ff0_0000_0000_0000 rn x = bfff00008000000000000000(-1) N ~Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:80 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of -1 rem -inf
frem.d [d]bff0_0000_0000_0000 fff0_0000_0000_0000 rn x = bfff00008000000000000000(-1) N ~Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of +0 rem +0
frem.d [d]0000_0000_0000_0000 0000_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of +0 rem -0
frem.d [d]0000_0000_0000_0000 8000_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of -0 rem +0
frem.d [d]8000_0000_0000_0000 0000_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of -0 rem -0
frem.d [d]8000_0000_0000_0000 8000_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of +0 rem +inf
frem.d [d]0000_0000_0000_0000 7ff0_0000_0000_0000 rn x = 000000000000000000000000(0) ~N Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of +0 rem -inf
frem.d [d]0000_0000_0000_0000 fff0_0000_0000_0000 rn x = 000000000000000000000000(0) ~N Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:80 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of -0 rem +inf
frem.d [d]8000_0000_0000_0000 7ff0_0000_0000_0000 rn x = 800000000000000000000000(0) N Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:80 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of -0 rem -inf
frem.d [d]8000_0000_0000_0000 fff0_0000_0000_0000 rn x = 800000000000000000000000(0) N Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of +inf rem +1
frem.d [d]7ff0_0000_0000_0000 3ff0_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of +inf rem -1
frem.d [d]7ff0_0000_0000_0000 bff0_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of -inf rem +1
frem.d [d]fff0_0000_0000_0000 3ff0_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of -inf rem -1
frem.d [d]fff0_0000_0000_0000 bff0_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of +inf rem +0
frem.d [d]7ff0_0000_0000_0000 0000_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of +inf rem -0
frem.d [d]7ff0_0000_0000_0000 8000_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of -inf rem +0
frem.d [d]fff0_0000_0000_0000 0000_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of -inf rem -0
frem.d [d]fff0_0000_0000_0000 8000_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of +inf rem +inf
frem.d [d]7ff0_0000_0000_0000 7ff0_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of +inf rem -inf
frem.d [d]7ff0_0000_0000_0000 fff0_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of -inf rem +inf
frem.d [d]fff0_0000_0000_0000 7ff0_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of -inf rem -inf
frem.d [d]fff0_0000_0000_0000 fff0_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP



#test frem of +0 rem +qnan
frem.d [d]0000_0000_0000_0000 7fff_ffff_ffff_ffff rn x = 7fff00007ffffffffffff800(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of +0 rem -qnan
frem.d [d]0000_0000_0000_0000 ffff_ffff_ffff_ffff rn x = ffff00007ffffffffffff800(-nan0xffffffff) N ~Z ~I NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of -0 rem +qnan
frem.d [d]8000_0000_0000_0000 7fff_ffff_ffff_ffff rn x = 7fff00007ffffffffffff800(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of -0 rem -qnan
frem.d [d]8000_0000_0000_0000 ffff_ffff_ffff_ffff rn x = ffff00007ffffffffffff800(-nan0xffffffff) N ~Z ~I NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of +qnan rem +0
frem.d [d]7fff_ffff_ffff_ffff 0000_0000_0000_0000 rn x = 7fff00007ffffffffffff800(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of -qnan rem +0
frem.d [d]ffff_ffff_ffff_ffff 0000_0000_0000_0000 rn x = ffff00007ffffffffffff800(-nan0xffffffff) N ~Z ~I NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of +qnan rem -0
frem.d [d]7fff_ffff_ffff_ffff 8000_0000_0000_0000 rn x = 7fff00007ffffffffffff800(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of -qnan rem -0
frem.d [d]ffff_ffff_ffff_ffff 8000_0000_0000_0000 rn x = ffff00007ffffffffffff800(-nan0xffffffff) N ~Z ~I NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of +snan rem +0
frem.d [d]7ff7_ffff_ffff_ffff 0000_0000_0000_0000 rn x snan = 7fff00007ffffffffffff800(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of +snan rem -0
frem.d [d]7ff7_ffff_ffff_ffff 8000_0000_0000_0000 rn x snan = 7fff00007ffffffffffff800(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of -snan rem +0
frem.d [d]fff7_ffff_ffff_ffff 0000_0000_0000_0000 rn x snan = ffff00007ffffffffffff800(-nan0xffffffff) N ~Z ~I NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of -snan rem -0
frem.d [d]fff7_ffff_ffff_ffff 8000_0000_0000_0000 rn x snan = ffff00007ffffffffffff800(-nan0xffffffff) N ~Z ~I NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of 1  positive denorm number rem +0
frem.d [d]0000_0000_0000_0001 0000_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of 1  negative denorm number rem -0
frem.d [d]8000_0000_0000_0001 8000_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of 1  positive denorm number rem +inf
frem.d [d]0000_0000_0000_0001 7ff0_0000_0000_0000 rn x = 3bcd00008000000000000000(0) ~N ~Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of 1  negative denorm number rem -inf
frem.d [d]8000_0000_0000_0001 fff0_0000_0000_0000 rn x = bbcd00008000000000000000(0) N ~Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of 1  positive denorm number rem 1 positive denorm number
frem.d [d]0000_0000_0000_0001 0000_0000_0000_0001 rn x = 000000000000000000000000(0) ~N Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:01 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test frem of 1  negative denorm number rem 1 negative denorm number
frem.d [d]8000_0000_0000_0001 8000_0000_0000_0001 rn x = 800000000000000000000000(0) N Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:01 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP
