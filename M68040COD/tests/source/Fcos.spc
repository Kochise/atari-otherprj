verbose
#TESTING FCOS	-- Cosine of source
#		-- Converts the source operand to extended precision (if necc-
#		-- ssary) and calculates the cosine of that number.  Store
#		-- the result in the destination floating point data register.
#		-- This function is not defined for source operands of + or -
#		-- infinity. If the source is not in the range of [-2pi...+2pi]
#		-- then the argument is reduced to within that range before
#		-- the cosine is calculated.  However, large arguments may
#		-- lose accuracy during reduction, and very large arguments (
#		-- greater than approximately 10 to the 20th) lose all 
#		-- accuracy.  The result is in the range of [-1...+1].


#TEST FCOS of +0

fcos.d 0000_0000_0000_0000 rn x

#TEST FCOS of -0

fcos.d 8000_0000_0000_0000 rn x

#TEST FCOS of Inf

fcos.d 7ff0_0000_0000_0000 rn x

#TEST FCOS of -Inf

fcos.d fff0_0000_0000_0000 rn x

#TEST FCOS of +QNAN

fcos.d 7fff_ffff_ffff_ffff rn x

#TEST FCOS of -QNAN

fcos.d ffff_ffff_ffff_ffff rn x

#TEST FCOS of +SNAN

fcos.d 7ff7_ffff_ffff_ffff rn x SNAN

#TEST FCOS of -SNAN

fcos.d fff7_ffff_ffff_ffff rn x SNAN

#TEST FCOS of positive Denorm numbers

fcos.d 0000_0000_0000_0001 rn x 
fcos.d 0008_0000_0000_0000 rn x 
fcos.d 0000_0000_8000_0000 rn x 

#TEST FCOS of negative Denorm numbers

fcos.d 8000_0000_0000_0001 rn x 
fcos.d 8008_0000_0000_0000 rn x 
fcos.d 8000_0000_8000_0000 rn x 
