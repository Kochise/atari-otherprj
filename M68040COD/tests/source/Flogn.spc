verbose
#TESTING FLOGN -- Log 3 of source
#		-- Converts the source operand to extended precision (if
#		-- necessary) and calculates the logarithm of that number
#		-- using base e arithmetic.  Stores the result in the
#		-- destination floating-point data register.  This function
#		-- is not defined for input values less than zero.


#TEST FLOGN of +0

flogn.d 0000_0000_0000_0000 rn x

#TEST FLOGN of -0

flogn.d 8000_0000_0000_0000 rn x

#TEST FLOGN of Inf

flogn.d 7ff0_0000_0000_0000 rn x

#TEST FLOGN of -Inf

flogn.d fff0_0000_0000_0000 rn x

#TEST FLOGN of +QNAN

flogn.d 7fff_ffff_ffff_ffff rn x

#TEST FLOGN of -QNAN

flogn.d ffff_ffff_ffff_ffff rn x

#TEST FLOGN of +SNAN

flogn.d 7ff7_ffff_ffff_ffff rn x SNAN

#TEST FLOGN of -SNAN

flogn.d fff7_ffff_ffff_ffff rn x SNAN

#TEST FLOGN of positive Denorm numbers

flogn.d 0000_0000_0000_0001 rn x 
flogn.d 0008_0000_0000_0000 rn x 
flogn.d 0000_0000_8000_0000 rn x 

#TEST FLOGN of negative Denorm numbers

flogn.d 8000_0000_0000_0001 rn x 
flogn.d 8008_0000_0000_0000 rn x 
flogn.d 8000_0000_8000_0000 rn x 
