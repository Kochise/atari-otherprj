verbose
#TESTING FATAN	-- Arc tangent of source
#		-- Converts the source operand to extended precision (if
#		-- necessary) and calculates the arc tangent of that number.
#		-- Stores the result in the destination floating point data
#		-- register.  The results is in the range of [-pi/2...+pi/2].

#TEST FATAN of +0

fatan.d 0000_0000_0000_0000 rn x

#TEST FATAN of -0

fatan.d 8000_0000_0000_0000 rn x

#TEST FATAN of Inf

fatan.d 7ff0_0000_0000_0000 rn x

#TEST FATAN of -Inf

fatan.d fff0_0000_0000_0000 rn x
#TEST FATAN of +QNAN

fatan.d 7fff_ffff_ffff_ffff rn x

#TEST FATAN of -QNAN

fatan.d ffff_ffff_ffff_ffff rn x

#TEST FATAN of +SNAN

fatan.d 7ff7_ffff_ffff_ffff rn x SNAN

#TEST FATAN of -SNAN

fatan.d fff7_ffff_ffff_ffff rn x SNAN

#TEST FATAN of positive Denorm numbers

fatan.d 0000_0000_0000_0001 
fatan.d 0008_0000_0000_0000
fatan.d 0000_0000_8000_0000

#TEST FATAN of negative Denorm numbers

fatan.d 8000_0000_0000_0001 
fatan.d 8008_0000_0000_0000
fatan.d 8000_0000_8000_0000
