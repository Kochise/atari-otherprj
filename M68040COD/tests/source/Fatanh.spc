verbose
#TESTING FATANH	-- Hyperbolic Arc Tangent of source
#		-- Converts the source operand to extended precision (if necc-
#		-- essary) and calculates the hyperbolic arc tangent of that
#		-- value.  Stores the result in the destination floating-
#		-- point register.  This function is not defined for source
#		-- operands outside of the range (-1...+1); and the result is
#		-- equal to -infinity or +infinity if the source is equal to
#		-- +1 or -1, respectively.  If the source is outside of the 
#		-- range [-1...+1], a NAN is returned as the result and the 
#		-- OPERR bit is set in the FPSR.

#TEST FATANH of +0

fatanh.d 0000_0000_0000_0000 rn x

#TEST FATANH of -0

fatanh.d 8000_0000_0000_0000 rn x

#TEST FATANH of Inf

fatanh.d 7ff0_0000_0000_0000 rn x

#TEST FATANH of -Inf

fatanh.d fff0_0000_0000_0000 rn x

#TEST FATANH of +QNAN

fatanh.d 7fff_ffff_ffff_ffff rn x

#TEST FATANH of -QNAN

fatanh.d ffff_ffff_ffff_ffff rn x

#TEST FATANH of +SNAN

fatanh.d 7ff7_ffff_ffff_ffff rn x SNAN

#TEST FATANH of -SNAN

fatanh.d fff7_ffff_ffff_ffff rn x SNAN

#TEST FATANH of positive Denorm numbers

fatanh.d 0000_0000_0000_0001 
fatanh.d 0008_0000_0000_0000
fatanh.d 0000_0000_8000_0000

#TEST FATANH of negative Denorm numbers

fatanh.d 8000_0000_0000_0001 
fatanh.d 8008_0000_0000_0000
fatanh.d 8000_0000_8000_0000
