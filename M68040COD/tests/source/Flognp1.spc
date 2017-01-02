verbose
#TESTING FLOGNP1 -- Log e of (source + 1)
#		-- Converts the source operand to extended precision (if
#		-- necessary), adds one to that value, and calculates the
#		-- natural logarithm of that intermediate result.  Stores the
#		-- result in the destination floating-point data register.
#		-- This function is not defined for input values less than -1.


#TEST FLOGNP1 of +0

flognp1.d 0000_0000_0000_0000 rn x

#TEST FLOGNP1 of -0

flognp1.d 8000_0000_0000_0000 rn x

#TEST FLOGNP1 of Inf

flognp1.d 7ff0_0000_0000_0000 rn x

#TEST FLOGNP1 of -Inf

flognp1.d fff0_0000_0000_0000 rn x

#TEST FLOGNP1 of +QNAN

flognp1.d 7fff_ffff_ffff_ffff rn x

#TEST FLOGNP1 of -QNAN

flognp1.d ffff_ffff_ffff_ffff rn x

#TEST FLOGNP1 of +SNAN

flognp1.d 7ff7_ffff_ffff_ffff rn x SNAN

#TEST FLOGNP1 of -SNAN

flognp1.d fff7_ffff_ffff_ffff rn x SNAN

#TEST FLOGNP1 of positive Denorm numbers

flognp1.d 0000_0000_0000_0001 rn x 
flognp1.d 0008_0000_0000_0000 rn x 
flognp1.d 0000_0000_8000_0000 rn x 

#TEST FLOGNP1 of negative Denorm numbers

flognp1.d 8000_0000_0000_0001 rn x 
flognp1.d 8008_0000_0000_0000 rn x 
flognp1.d 8000_0000_8000_0000 rn x 
