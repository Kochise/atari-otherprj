verbose
#TESTING FASIN	-- Arc sine of source
#		-- Converts the source operand to extended precision (if necc-
#		-- ssary) and calculates the arc sine of that number.  Store
#		-- the result in the destination floating point data register.
#		-- This function is not defined for source operands outside of
#		-- the range [-1...+1]; if the source is not in the correct 
#		-- range, a NAN is returned as the result and the OPERR bit is
#		-- set in the FPSR.  If the source is in the correct range, the
#		-- result is in the range of [-pi/2...+pi/2].

#TEST FASIN of +0

fasin.d 0000_0000_0000_0000 rn x

#TEST FASIN of -0

fasin.d 8000_0000_0000_0000 rn x

#TEST FASIN of Inf

fasin.d 7ff0_0000_0000_0000 rn x

#TEST FASIN of -Inf

fasin.d fff0_0000_0000_0000 rn x

#TEST FASIN of +QNAN

fasin.d 7fff_ffff_ffff_ffff rn x

#TEST FASIN of -QNAN

fasin.d ffff_ffff_ffff_ffff rn x

#TEST FASIN of +SNAN

fasin.d 7ff7_ffff_ffff_ffff rn x SNAN

#TEST FASIN of -SNAN

fasin.d fff7_ffff_ffff_ffff rn x SNAN

#TEST FASIN of 1  positive Denorm number

fasin.d 0000_0000_0000_0001 rn x
fasin.d 0008_0000_0000_0000 rn x 
fasin.d 0000_0000_8000_0000 rn x 

#TEST FASIN of 1  negative Denorm number

fasin.d 8000_0000_0000_0001 rn x
fasin.d 8008_0000_0000_0000 rn x 
fasin.d 8000_0000_8000_0000 rn x 
