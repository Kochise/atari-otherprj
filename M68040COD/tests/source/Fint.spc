verbose
#TESTING FINT   -- Integer Part
#		-- Converts the source operand to extended precision (if
#		-- necessary) and extracts the integer part and converts it to
#		-- an extended precision floating-point number.  Stores the
#		-- result in the destination floating-point data register.
#		-- The integer part is extracted by rounding the extended
#		-- precision number to an integer using the current rounding
#		-- mode selected in the FPCR mode control byte.  Thus, the
#		-- integer part returned is the number that is to the left
#		-- of the radix point when the exponent is zero, after
#		-- rounding.  For example, the integer part of 137.57 is 137.0
#		-- for the round-to-zero and round-to-minus infinity modes, 
#		-- and 138.0 for the round-to-nearest and round-to-plus-
#		-- infinity modes.  Note that the result of this operation is
#		-- a floating-point number.


#TEST FINT of +0

fint.d 0000_0000_0000_0000 rn x

#TEST FINT of -0

fint.d 8000_0000_0000_0000 rn x

#TEST FINT of Inf

fint.d 7ff0_0000_0000_0000 rn x

#TEST FINT of -Inf

fint.d fff0_0000_0000_0000 rn x

#TEST FINT of +QNAN

fint.d 7fff_ffff_ffff_ffff rn x

#TEST FINT of -QNAN

fint.d ffff_ffff_ffff_ffff rn x

#TEST FINT of +SNAN

fint.d 7ff7_ffff_ffff_ffff rn x SNAN

#TEST FINT of -SNAN

fint.d fff7_ffff_ffff_ffff rn x SNAN

#TEST FINT of positive Denorm numbers

fint.d 0000_0000_0000_0001 rn x 
fint.d 0008_0000_0000_0000 rn x 
fint.d 0000_0000_8000_0000 rn x 

#TEST FINT of negative Denorm numbers

fint.d 8000_0000_0000_0001 rn x 
fint.d 8008_0000_0000_0000 rn x 
fint.d 8000_0000_8000_0000 rn x 
