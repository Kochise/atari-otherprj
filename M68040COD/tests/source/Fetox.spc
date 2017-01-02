verbose
#TESTING FETOX	-- e to the (Source)power
#		-- Converts the source operand to extended precision (if
#		-- necessary) and calculates e to the power of that number.
#		-- Stores the result in the destination floating-point data
#		-- register.


#TEST FETOX of +0

fetox.d 0000_0000_0000_0000 rn x

#TEST FETOX of -0

fetox.d 8000_0000_0000_0000 rn x

#TEST FETOX of Inf

fetox.d 7ff0_0000_0000_0000 rn x

#TEST FETOX of -Inf

fetox.d fff0_0000_0000_0000 rn x

#TEST FETOX of +QNAN

fetox.d 7fff_ffff_ffff_ffff rn x

#TEST FETOX of -QNAN

fetox.d ffff_ffff_ffff_ffff rn x

#TEST FETOX of +SNAN

fetox.d 7ff7_ffff_ffff_ffff rn x SNAN

#TEST FETOX of -SNAN

fetox.d fff7_ffff_ffff_ffff rn x SNAN

#TEST FETOX of positive Denorm numbers

fetox.d 0000_0000_0000_0001 rn x 
fetox.d 0008_0000_0000_0000 rn x 
fetox.d 0000_0000_8000_0000 rn x 

#TEST FETOX of negative Denorm numbers

fetox.d 8000_0000_0000_0001 rn x 
fetox.d 8008_0000_0000_0000 rn x 
fetox.d 8000_0000_8000_0000 rn x 
