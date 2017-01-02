verbose
#TESTING FACOS	-- Arc cosine of source
#		-- Converts the source operand to extended precision (if necc-
#		-- ssary) and calculates the arc cosine of that number.  Store
#		-- the result in the destination floating point data register.
#		-- This function is not defined for source operands outside of
#		-- the range [-1...+1]; if the source is not in the correct 
#		-- range, a NAN is returned as the result and the OPERR bit is
#		-- set in the FPSR.  If the source is in the correct range, the
#		-- result is in the range of [0...pi].

#TEST FACOS of +0

facos.d 0000_0000_0000_0000 rn x

#TEST FACOS of -0

facos.d 8000_0000_0000_0000 rn x

#TEST FACOS of Inf

facos.d 7ff0_0000_0000_0000 rn x

#TEST FACOS of -Inf

facos.d fff0_0000_0000_0000 rn x

#TEST FACOS of +QNAN

facos.d 7fff_ffff_ffff_ffff rn x

#TEST FACOS of -QNAN

facos.d ffff_ffff_ffff_ffff rn x

#TEST FACOS of +SNAN

facos.d 7ff7_ffff_ffff_ffff rn x SNAN

#TEST FACOS of -SNAN

facos.d fff7_ffff_ffff_ffff rn x SNAN

#TEST FACOS of positive Denorm numbers

facos.d 0000_0000_0000_0001 rn x 
facos.d 0008_0000_0000_0000 rn x 
facos.d 0000_0000_8000_0000 rn x 

#TEST FACOS of negative Denorm numbers

facos.d 8000_0000_0000_0001 rn x 
facos.d 8008_0000_0000_0000 rn x 
facos.d 8000_0000_8000_0000 rn x 
