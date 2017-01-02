verbose
#TESTING FTENTOX-- 10 to the (Source)power
#		-- Converts the source operand to extended precision (if
#		-- necessary) and calculates 10 to the power of that number.
#		-- Stores the result in the destination floating-point data
#		-- register.


#TEST FTENTOX of +0

ftentox.d 0000_0000_0000_0000 rn x

#TEST FTENTOX of -0

ftentox.d 8000_0000_0000_0000 rn x

#TEST FTENTOX of Inf

ftentox.d 7ff0_0000_0000_0000 rn x

#TEST FTENTOX of -Inf

ftentox.d fff0_0000_0000_0000 rn x

#TEST FTENTOX of +QNAN

ftentox.d 7fff_ffff_ffff_ffff rn x

#TEST FTENTOX of -QNAN

ftentox.d ffff_ffff_ffff_ffff rn x

#TEST FTENTOX of +SNAN

ftentox.d 7ff7_ffff_ffff_ffff rn x SNAN

#TEST FTENTOX of -SNAN

ftentox.d fff7_ffff_ffff_ffff rn x SNAN

#TEST FTENTOX of positive Denorm numbers

ftentox.d 0000_0000_0000_0001 rn x 
ftentox.d 0008_0000_0000_0000 rn x 
ftentox.d 0000_0000_8000_0000 rn x 

#TEST FTENTOX of negative Denorm numbers

ftentox.d 8000_0000_0000_0001 rn x 
ftentox.d 8008_0000_0000_0000 rn x 
ftentox.d 8000_0000_8000_0000 rn x 
