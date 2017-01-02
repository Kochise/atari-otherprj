verbose
#TESTING FTAN	-- Tangent
#		-- Converts the source operand to extended precision ( if
#		-- necessary) and calculates the tangent of that
#		-- number.  Stores the result in the destination floating-point
#		-- data register. This function is not defined for source
#		-- operands of + or - infinity.  If the source operand is not
#		-- in the range of [-pi/2...+pi/2], the argument is reduced to
#		-- within that range before the tangent is calculated.
#		-- However, large arguments may lose accuracy during reduction,
#		-- and very large arguments (greater than approximately 10 to
#		-- the 20th ) lose all accuracy.

#TEST FTAN of +0

ftan.d 0000_0000_0000_0000 rn x

#TEST FTAN of -0

ftan.d 8000_0000_0000_0000 rn x

#TEST FTAN of Inf

ftan.d 7ff0_0000_0000_0000 rn x

#TEST FTAN of -Inf

ftan.d fff0_0000_0000_0000 rn x

#TEST FTAN of +QNAN

ftan.d 7fff_ffff_ffff_ffff rn x

#TEST FTAN of -QNAN

ftan.d ffff_ffff_ffff_ffff rn x

#TEST FTAN of +SNAN

ftan.d 7ff7_ffff_ffff_ffff rn x SNAN

#TEST FTAN of -SNAN

ftan.d fff7_ffff_ffff_ffff rn x SNAN

#TEST FTAN of positive Denorm numbers

ftan.d 0000_0000_0000_0001 rn x 
ftan.d 0008_0000_0000_0000 rn x 
ftan.d 0000_0000_8000_0000 rn x 

#TEST FTAN of negative Denorm numbers

ftan.d 8000_0000_0000_0001 rn x 
ftan.d 8008_0000_0000_0000 rn x 
ftan.d 8000_0000_8000_0000 rn x 
