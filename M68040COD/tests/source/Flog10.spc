verbose
#TESTING FLOG10 -- Log 10 of source
#		-- Converts the source operand to extended precision (if
#		-- necessary) and calculates the logarithm of that number
#		-- using base 10 arithmetic.  Stores the result in the
#		-- destination floating-point data register.  This function
#		-- is not defined for input values less than zero.


#TEST FLOG10 of +0

flog10.d 0000_0000_0000_0000 rn x

#TEST FLOG10 of -0

flog10.d 8000_0000_0000_0000 rn x

#TEST FLOG10 of Inf

flog10.d 7ff0_0000_0000_0000 rn x

#TEST FLOG10 of -Inf

flog10.d fff0_0000_0000_0000 rn x

#TEST FLOG10 of +QNAN

flog10.d 7fff_ffff_ffff_ffff rn x

#TEST FLOG10 of -QNAN

flog10.d ffff_ffff_ffff_ffff rn x

#TEST FLOG10 of +SNAN

flog10.d 7ff7_ffff_ffff_ffff rn x SNAN

#TEST FLOG10 of -SNAN

flog10.d fff7_ffff_ffff_ffff rn x SNAN

#TEST FLOG10 of positive Denorm numbers

flog10.d 0000_0000_0000_0001 rn x 
flog10.d 0008_0000_0000_0000 rn x 
flog10.d 0000_0000_8000_0000 rn x 

#TEST FLOG10 of negative Denorm numbers

flog10.d 8000_0000_0000_0001 rn x 
flog10.d 8008_0000_0000_0000 rn x 
flog10.d 8000_0000_8000_0000 rn x 
