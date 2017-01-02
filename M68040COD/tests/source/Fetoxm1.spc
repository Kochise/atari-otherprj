verbose
#TESTING FETOXM1-- e to the (Source)power -1
#		-- Converts the source operand to extended precision (if
#		-- necessary) and calculates e to the power of that number.
#		-- Then, subtracts one from that value.
#		-- Stores the result in the destination floating-point data
#		-- register.


#TEST FETOXM1 of +0

fetoxm1.d 0000_0000_0000_0000 rn x

#TEST FETOXM1 of -0

fetoxm1.d 8000_0000_0000_0000 rn x

#TEST FETOXM1 of Inf

fetoxm1.d 7ff0_0000_0000_0000 rn x

#TEST FETOXM1 of -Inf

fetoxm1.d fff0_0000_0000_0000 rn x

#TEST FETOXM1 of +QNAN

fetoxm1.d 7fff_ffff_ffff_ffff rn x

#TEST FETOXM1 of -QNAN

fetoxm1.d ffff_ffff_ffff_ffff rn x

#TEST FETOXM1 of +SNAN

fetoxm1.d 7ff7_ffff_ffff_ffff rn x SNAN

#TEST FETOXM1 of -SNAN

fetoxm1.d fff7_ffff_ffff_ffff rn x SNAN

#TEST FETOXM1 of positive Denorm numbers

fetoxm1.d 0000_0000_0000_0001 rn x 
fetoxm1.d 0008_0000_0000_0000 rn x 
fetoxm1.d 0000_0000_8000_0000 rn x 

#TEST FETOXM1 of negative Denorm numbers

fetoxm1.d 8000_0000_0000_0001 rn x 
fetoxm1.d 8008_0000_0000_0000 rn x 
fetoxm1.d 8000_0000_8000_0000 rn x 
