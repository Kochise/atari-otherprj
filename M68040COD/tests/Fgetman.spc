#testing fgetman-- get mantissa
#		-- converts the source operand to extended precision (if
#		-- necessary) and extracts the mantissa.  converts the mantissa
#		-- to an extended precision value and stores the result in the
#		-- destination floating-point data register.  the result is in
#		-- the range [1.0...2.0) with the sign of the source mantissa,
#		-- zero, or is a nan. 


#test fgetman of +0

fgetman.d 0000_0000_0000_0000 rn x = 000000000000000000000000(0) ~N Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fgetman of -0

fgetman.d 8000_0000_0000_0000 rn x = 800000000000000000000000(0) N Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fgetman of inf

fgetman.d 7ff0_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fgetman of -inf

fgetman.d fff0_0000_0000_0000 rn x = 7fff0000ffffffffffffffff(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fgetman of +qnan

fgetman.d 7fff_ffff_ffff_ffff rn x = 7fff00007ffffffffffff800(nan0xffffffff) ~N ~Z ~I NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fgetman of -qnan

fgetman.d ffff_ffff_ffff_ffff rn x = ffff00007ffffffffffff800(-nan0xffffffff) N ~Z ~I NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fgetman of +snan

fgetman.d 7ff7_ffff_ffff_ffff rn x snan = 7fff0000c0ffeefacadec0da(nan0x81ffddf5) ~N ~Z ~I NAN ~BSUN SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL TNAN ~TUNIMP ~TUNSUP

#test fgetman of -snan

fgetman.d fff7_ffff_ffff_ffff rn x snan = 7fff0000c0ffeefacadec0da(nan0x81ffddf5) N ~Z ~I NAN ~BSUN SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL TNAN ~TUNIMP ~TUNSUP

#test fgetman of positive denorm numbers

fgetman.d 0000_0000_0000_0001 rn x  = 3fff00008000000000000000(1) ~N ~Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP
fgetman.d 0008_0000_0000_0000 rn x  = 3fff00008000000000000000(1) ~N ~Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP
fgetman.d 0000_0000_8000_0000 rn x  = 3fff00008000000000000000(1) ~N ~Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP

#test fgetman of negative denorm numbers

fgetman.d 8000_0000_0000_0001 rn x  = bfff00008000000000000000(-1) N ~Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP
fgetman.d 8008_0000_0000_0000 rn x  = bfff00008000000000000000(-1) N ~Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP
fgetman.d 8000_0000_8000_0000 rn x  = bfff00008000000000000000(-1) N ~Z ~I ~NAN ~BSUN ~SNAN ~OPERR ~OVFL ~UNFL ~DZ ~INEX2 ~INEX1 ~AIOP ~AOVFL ~AUNFL ~ADZ ~AINEX Q:00 ~T ~SIGFPE ~TBSUN ~TINEX ~TDZ ~TUNFL ~TOPERR ~TOVFL ~TNAN ~TUNIMP ~TUNSUP
