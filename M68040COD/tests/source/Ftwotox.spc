verbose
#TESTING FTWOTOX-- 2 to the (Source)power
#		-- Converts the source operand to extended precision (if
#		-- necessary) and calculates 2 to the power of that number.
#		-- Stores the result in the destination floating-point data
#		-- register.


#TEST FTWOTOX of +0

ftwotox.d 0000_0000_0000_0000 rn x

#TEST FTWOTOX of -0

ftwotox.d 8000_0000_0000_0000 rn x

#TEST FTWOTOX of Inf

ftwotox.d 7ff0_0000_0000_0000 rn x

#TEST FTWOTOX of -Inf

ftwotox.d fff0_0000_0000_0000 rn x

#TEST FTWOTOX of +QNAN

ftwotox.d 7fff_ffff_ffff_ffff rn x

#TEST FTWOTOX of -QNAN

ftwotox.d ffff_ffff_ffff_ffff rn x

#TEST FTWOTOX of +SNAN

ftwotox.d 7ff7_ffff_ffff_ffff rn x SNAN

#TEST FTWOTOX of -SNAN

ftwotox.d fff7_ffff_ffff_ffff rn x SNAN

#TEST FTWOTOX of positive Denorm numbers

ftwotox.d 0000_0000_0000_0001 rn x 
ftwotox.d 0008_0000_0000_0000 rn x 
ftwotox.d 0000_0000_8000_0000 rn x 

#TEST FTWOTOX of negative Denorm numbers

ftwotox.d 8000_0000_0000_0001 rn x 
ftwotox.d 8008_0000_0000_0000 rn x 
ftwotox.d 8000_0000_8000_0000 rn x 
