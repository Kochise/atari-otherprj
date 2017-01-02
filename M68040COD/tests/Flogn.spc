#testing flogn -- log 3 of source
#		-- converts the source operand to extended precision (if
#		-- necessary) and calculates the logarithm of that number
#		-- using base e arithmetic.  stores the result in the
#		-- destination floating-point data register.  this function
#		-- is not defined for input values less than zero.


#test flogn of +0

flogn.d 0000_0000_0000_0000 rn x = ffff00000000000000000000(-inf) N ~Z I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test flogn of -0

flogn.d 8000_0000_0000_0000 rn x = ffff00000000000000000000(-inf) N ~Z I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test flogn of inf

flogn.d 7ff0_0000_0000_0000 rn x = 7fff00000000000000000000(inf) ~N ~Z I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test flogn of -inf

flogn.d fff0_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test flogn of +qnan

flogn.d 7fff_ffff_ffff_ffff rn x = 7fff00007ffffffffffff800(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test flogn of -qnan

flogn.d ffff_ffff_ffff_ffff rn x = ffff00007ffffffffffff800(-nan0xffffffff) N ~Z ~I NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test flogn of +snan

flogn.d 7ff7_ffff_ffff_ffff rn x snan = 7fff0000c0ffeefacadec0da(nan0x81ffddf5) ~N ~Z ~I NAN ~BSUN SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL TNAN ~TUNIMP ~TUNSUP

#test flogn of -snan

flogn.d fff7_ffff_ffff_ffff rn x snan = 7fff0000c0ffeefacadec0da(nan0x81ffddf5) N ~Z ~I NAN ~BSUN SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL TNAN ~TUNIMP ~TUNSUP

#test flogn of positive denorm numbers

flogn.d 0000_0000_0000_0001 rn x  = c0080000ba1c2a236b8e1b1d(-744.44) N ~Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP
flogn.d 0008_0000_0000_0000 rn x  = c0080000b145bb71d3db05ce(-709.09) N ~Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP
flogn.d 0000_0000_8000_0000 rn x  = c0080000b4bcf5e9aaf4132e(-722.953) N ~Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test flogn of negative denorm numbers

flogn.d 8000_0000_0000_0001 rn x  = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP
flogn.d 8008_0000_0000_0000 rn x  = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP
flogn.d 8000_0000_8000_0000 rn x  = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP
