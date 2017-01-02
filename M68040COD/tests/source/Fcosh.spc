verbose
#TESTING FCOSH	-- Hyperbolic Cosine of source
#		-- Converts the source operand to extended precision (if necc-
#		-- ary) and calculates the hyperbolic cosine of that number.
#		-- Stores the result in the destination floating-point data
#		-- register.


#TEST FCOSH of +0

fcosh.d 0000_0000_0000_0000 rn x

#TEST FCOSH of -0

fcosh.d 8000_0000_0000_0000 rn x

#TEST FCOSH of Inf

fcosh.d 7ff0_0000_0000_0000 rn x

#TEST FCOSH of -Inf

fcosh.d fff0_0000_0000_0000 rn x

#TEST FCOSH of +QNAN

fcosh.d 7fff_ffff_ffff_ffff rn x

#TEST FCOSH of -QNAN

fcosh.d ffff_ffff_ffff_ffff rn x

#TEST FCOSH of +SNAN

fcosh.d 7ff7_ffff_ffff_ffff rn x SNAN

#TEST FCOSH of -SNAN

fcosh.d fff7_ffff_ffff_ffff rn x SNAN

#TEST FCOSH of positive Denorm numbers

fcosh.d 0000_0000_0000_0001 rn x 
fcosh.d 0008_0000_0000_0000 rn x 
fcosh.d 0000_0000_8000_0000 rn x 

#TEST FCOSH of negative Denorm numbers

fcosh.d 8000_0000_0000_0001 rn x 
fcosh.d 8008_0000_0000_0000 rn x 
fcosh.d 8000_0000_8000_0000 rn x 
