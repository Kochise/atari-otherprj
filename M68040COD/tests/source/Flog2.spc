verbose
#TESTING FLOG2 -- Log 2 of source
#		-- Converts the source operand to extended precision (if
#		-- necessary) and calculates the logarithm of that number
#		-- using base 2 arithmetic.  Stores the result in the
#		-- destination floating-point data register.  This function
#		-- is not defined for input values less than zero.


#TEST FLOG2 of +0

flog2.d 0000_0000_0000_0000 rn x

#TEST FLOG2 of -0

flog2.d 8000_0000_0000_0000 rn x

#TEST FLOG2 of Inf

flog2.d 7ff0_0000_0000_0000 rn x

#TEST FLOG2 of -Inf

flog2.d fff0_0000_0000_0000 rn x

#TEST FLOG2 of +QNAN

flog2.d 7fff_ffff_ffff_ffff rn x

#TEST FLOG2 of -QNAN

flog2.d ffff_ffff_ffff_ffff rn x

#TEST FLOG2 of +SNAN

flog2.d 7ff7_ffff_ffff_ffff rn x SNAN

#TEST FLOG2 of -SNAN

flog2.d fff7_ffff_ffff_ffff rn x SNAN

#TEST FLOG2 of positive Denorm numbers

flog2.d 0000_0000_0000_0001 rn x 
flog2.d 0008_0000_0000_0000 rn x 
flog2.d 0000_0000_8000_0000 rn x 

#TEST FLOG2 of negative Denorm numbers

flog2.d 8000_0000_0000_0001 rn x 
flog2.d 8008_0000_0000_0000 rn x 
flog2.d 8000_0000_8000_0000 rn x 
