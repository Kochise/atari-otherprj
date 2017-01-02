verbose
#TESTING FMOD	-- Modulo Remainder
#		-- Converts the source operand to extended precision i(if
#		-- necessary) and calculates the modulo remainder of the number
#		-- in the destination floating-point data register, using the
#		-- the source operand as the modulus.  Stores the result in
#		-- the destination floating-point data register, and stores 
#		-- the sign and seven least significant bits of the quotient
#		-- in the FPSR quotient byte(the quotient is the result of
#		-- FPn(/) Source).  The modulo remainder function is defined as:
#		-- 	FPn -(Source X N) 
#		-- where:
#		--    	N=INT(FPn(/)Source) in the the round-to-zero mode
#		-- The FMOD function is not defined for a source operand equal
#		-- to zero or for a destination operand equal to infinity. Note
#		-- that this function is not the same as the FREM instruction,
#		-- which uses the round-to-nearest mode and thus returns the 
#		-- remainder that is required by the IEEE Specification for
#		-- Binary Floating-Point Arithmetic.


#TEST FMOD of +1 mod +inf
fmod.d [d]3ff0_0000_0000_0000 7ff0_0000_0000_0000 rn x


#TEST FMOD of +1 mod -inf
fmod.d [d]3ff0_0000_0000_0000 fff0_0000_0000_0000 rn x


#TEST FMOD of -1 mod +inf
fmod.d [d]bff0_0000_0000_0000 7ff0_0000_0000_0000 rn x

#TEST FMOD of -1 mod -inf
fmod.d [d]bff0_0000_0000_0000 fff0_0000_0000_0000 rn x

#TEST FMOD of +0 mod +0
fmod.d [d]0000_0000_0000_0000 0000_0000_0000_0000 rn x

#TEST FMOD of +0 mod -0
fmod.d [d]0000_0000_0000_0000 8000_0000_0000_0000 rn x

#TEST FMOD of -0 mod +0
fmod.d [d]8000_0000_0000_0000 0000_0000_0000_0000 rn x

#TEST FMOD of -0 mod -0
fmod.d [d]8000_0000_0000_0000 8000_0000_0000_0000 rn x

#TEST FMOD of +0 mod +Inf
fmod.d [d]0000_0000_0000_0000 7ff0_0000_0000_0000 rn x

#TEST FMOD of +0 mod -Inf
fmod.d [d]0000_0000_0000_0000 fff0_0000_0000_0000 rn x

#TEST FMOD of -0 mod +Inf
fmod.d [d]8000_0000_0000_0000 7ff0_0000_0000_0000 rn x

#TEST FMOD of -0 mod -Inf
fmod.d [d]8000_0000_0000_0000 fff0_0000_0000_0000 rn x

#TEST FMOD of +Inf mod +1
fmod.d [d]7ff0_0000_0000_0000 3ff0_0000_0000_0000 rn x

#TEST FMOD of +Inf mod -1
fmod.d [d]7ff0_0000_0000_0000 bff0_0000_0000_0000 rn x

#TEST FMOD of -Inf mod +1
fmod.d [d]fff0_0000_0000_0000 3ff0_0000_0000_0000 rn x

#TEST FMOD of -Inf mod -1
fmod.d [d]fff0_0000_0000_0000 bff0_0000_0000_0000 rn x

#TEST FMOD of +Inf mod +0
fmod.d [d]7ff0_0000_0000_0000 0000_0000_0000_0000 rn x

#TEST FMOD of +Inf mod -0
fmod.d [d]7ff0_0000_0000_0000 8000_0000_0000_0000 rn x

#TEST FMOD of -Inf mod +0
fmod.d [d]fff0_0000_0000_0000 0000_0000_0000_0000 rn x

#TEST FMOD of -Inf mod -0
fmod.d [d]fff0_0000_0000_0000 8000_0000_0000_0000 rn x

#TEST FMOD of +Inf mod +Inf
fmod.d [d]7ff0_0000_0000_0000 7ff0_0000_0000_0000 rn x

#TEST FMOD of +Inf mod -Inf
fmod.d [d]7ff0_0000_0000_0000 fff0_0000_0000_0000 rn x

#TEST FMOD of -Inf mod +Inf
fmod.d [d]fff0_0000_0000_0000 7ff0_0000_0000_0000 rn x

#TEST FMOD of -Inf mod -Inf
fmod.d [d]fff0_0000_0000_0000 fff0_0000_0000_0000 rn x



#TEST FMOD of +0 mod +QNAN
fmod.d [d]0000_0000_0000_0000 7fff_ffff_ffff_ffff rn x

#TEST FMOD of +0 mod -QNAN
fmod.d [d]0000_0000_0000_0000 ffff_ffff_ffff_ffff rn x

#TEST FMOD of -0 mod +QNAN
fmod.d [d]8000_0000_0000_0000 7fff_ffff_ffff_ffff rn x

#TEST FMOD of -0 mod -QNAN
fmod.d [d]8000_0000_0000_0000 ffff_ffff_ffff_ffff rn x

#TEST FMOD of +QNAN mod +0
fmod.d [d]7fff_ffff_ffff_ffff 0000_0000_0000_0000 rn x

#TEST FMOD of -QNAN mod +0
fmod.d [d]ffff_ffff_ffff_ffff 0000_0000_0000_0000 rn x

#TEST FMOD of +QNAN mod -0
fmod.d [d]7fff_ffff_ffff_ffff 8000_0000_0000_0000 rn x

#TEST FMOD of -QNAN mod -0
fmod.d [d]ffff_ffff_ffff_ffff 8000_0000_0000_0000 rn x

#TEST FMOD of +SNAN mod +0
fmod.d [d]7ff7_ffff_ffff_ffff 0000_0000_0000_0000 rn x SNAN

#TEST FMOD of +SNAN mod -0
fmod.d [d]7ff7_ffff_ffff_ffff 8000_0000_0000_0000 rn x SNAN

#TEST FMOD of -SNAN mod +0
fmod.d [d]fff7_ffff_ffff_ffff 0000_0000_0000_0000 rn x SNAN

#TEST FMOD of -SNAN mod -0
fmod.d [d]fff7_ffff_ffff_ffff 8000_0000_0000_0000 rn x SNAN

#TEST FMOD of 1  positive Denorm number mod +0
fmod.d [d]0000_0000_0000_0001 0000_0000_0000_0000 rn x

#TEST FMOD of 1  negative Denorm number mod -0
fmod.d [d]8000_0000_0000_0001 8000_0000_0000_0000 rn x

#TEST FMOD of 1  positive Denorm number mod +Inf
fmod.d [d]0000_0000_0000_0001 7ff0_0000_0000_0000 rn x

#TEST FMOD of 1  negative Denorm number mod -Inf
fmod.d [d]8000_0000_0000_0001 fff0_0000_0000_0000 rn x

#TEST FMOD of 1  positive Denorm number mod 1 positive Denorm number
fmod.d [d]0000_0000_0000_0001 0000_0000_0000_0001 rn x

#TEST FMOD of 1  negative Denorm number mod 1 negative Denorm number
fmod.d [d]8000_0000_0000_0001 8000_0000_0000_0001 rn x
