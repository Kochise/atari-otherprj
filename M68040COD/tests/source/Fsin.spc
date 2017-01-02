verbose
#TESTING FSIN	-- Sine of Source
#		-- Converts the source operand to extended precision ( if
#		-- necessary) and calculates the sine of that number.  Stores
#		-- the result in the destination floating-point data register.
#		-- This function is not defined for source operands of + or -
#		-- infinity.  If the source operand is not in the range of 
#		-- [-2pi...+2pi], the argument is reduced to within that range
#		-- before the sine is calculated.  However, large arguments
#		-- may lose accuracey during reduction, and very large 
#		-- arguments (greater than approximately 10 to the 20th) lose
#		-- all accuracy.  The result is in the range of [-1...+1]

#TEST FSIN of +0

fsin.d 0000_0000_0000_0000 rn x

#TEST FSIN of -0

fsin.d 8000_0000_0000_0000 rn x

#TEST FSIN of Inf

fsin.d 7ff0_0000_0000_0000 rn x

#TEST FSIN of -Inf

fsin.d fff0_0000_0000_0000 rn x

#TEST FSIN of +QNAN

fsin.d 7fff_ffff_ffff_ffff rn x

#TEST FSIN of -QNAN

fsin.d ffff_ffff_ffff_ffff rn x

#TEST FSIN of +SNAN

fsin.d 7ff7_ffff_ffff_ffff rn x SNAN

#TEST FSIN of -SNAN

fsin.d fff7_ffff_ffff_ffff rn x SNAN

#TEST FSIN of positive Denorm numbers

fsin.d 0000_0000_0000_0001 rn x 
fsin.d 0008_0000_0000_0000 rn x 
fsin.d 0000_0000_8000_0000 rn x 

#TEST FSIN of negative Denorm numbers

fsin.d 8000_0000_0000_0001 rn x 
fsin.d 8008_0000_0000_0000 rn x 
fsin.d 8000_0000_8000_0000 rn x 
