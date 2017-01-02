#testing fatanh	-- hyperbolic arc tangent of source
#		-- converts the source operand to extended precision (if necc-
#		-- essary) and calculates the hyperbolic arc tangent of that
#		-- value.  stores the result in the destination floating-
#		-- point register.  this function is not defined for source
#		-- operands outside of the range (-1...+1); and the result is
#		-- equal to -infinity or +infinity if the source is equal to
#		-- +1 or -1, respectively.  if the source is outside of the 
#		-- range [-1...+1], a nan is returned as the result and the 
#		-- operr bit is set in the fpsr.

#test fatanh of +0

fatanh.d 0000_0000_0000_0000 rn x = 000000000000000000000000(0) ~N Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fatanh of -0

fatanh.d 8000_0000_0000_0000 rn x = 800000000000000000000000(0) N Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fatanh of inf

fatanh.d 7ff0_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fatanh of -inf

fatanh.d fff0_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fatanh of +qnan

fatanh.d 7fff_ffff_ffff_ffff rn x = 7fff00007ffffffffffff800(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fatanh of -qnan

fatanh.d ffff_ffff_ffff_ffff rn x = ffff00007ffffffffffff800(-nan0xffffffff) N ~Z ~I NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fatanh of +snan

fatanh.d 7ff7_ffff_ffff_ffff rn x snan = 7fff0000c0ffeefacadec0da(nan0x81ffddf5) ~N ~Z ~I NAN ~BSUN SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL TNAN ~TUNIMP ~TUNSUP

#test fatanh of -snan

fatanh.d fff7_ffff_ffff_ffff rn x snan = 7fff0000c0ffeefacadec0da(nan0x81ffddf5) N ~Z ~I NAN ~BSUN SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL TNAN ~TUNIMP ~TUNSUP

#test fatanh of positive denorm numbers

fatanh.d 0000_0000_0000_0001  = 3bcd00008000000000000000(0) ~N ~Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP
fatanh.d 0008_0000_0000_0000 = 3c0000008000000000000000(0) ~N ~Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP
fatanh.d 0000_0000_8000_0000 = 3bec00008000000000000000(0) ~N ~Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fatanh of negative denorm numbers

fatanh.d 8000_0000_0000_0001  = bbcd00008000000000000000(0) N ~Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP
fatanh.d 8008_0000_0000_0000 = bc0000008000000000000000(0) N ~Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP
fatanh.d 8000_0000_8000_0000 = bbec00008000000000000000(0) N ~Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP
