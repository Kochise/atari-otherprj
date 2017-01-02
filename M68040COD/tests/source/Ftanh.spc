verbose
#TESTING FTANH	-- Hyperbolic Tangent
#		-- Converts the source operand to extended precision ( if
#		-- necessary) and calculates the hyperbolic tangent of that
#		-- number.  Stores the result in the destination floating-
#		-- point data register.

#TEST FTANH of +0

ftanh.d 0000_0000_0000_0000 rn x

#TEST FTANH of -0

ftanh.d 8000_0000_0000_0000 rn x

#TEST FTANH of Inf

ftanh.d 7ff0_0000_0000_0000 rn x

#TEST FTANH of -Inf

ftanh.d fff0_0000_0000_0000 rn x

#TEST FTANH of +QNAN

ftanh.d 7fff_ffff_ffff_ffff rn x

#TEST FTANH of -QNAN

ftanh.d ffff_ffff_ffff_ffff rn x

#TEST FTANH of +SNAN

ftanh.d 7ff7_ffff_ffff_ffff rn x SNAN

#TEST FTANH of -SNAN

ftanh.d fff7_ffff_ffff_ffff rn x SNAN

#TEST FTANH of positive Denorm numbers

ftanh.d 0000_0000_0000_0001 rn x 
ftanh.d 0008_0000_0000_0000 rn x 
ftanh.d 0000_0000_8000_0000 rn x 

#TEST FTANH of negative Denorm numbers

ftanh.d 8000_0000_0000_0001 rn x 
ftanh.d 8008_0000_0000_0000 rn x 
ftanh.d 8000_0000_8000_0000 rn x 
