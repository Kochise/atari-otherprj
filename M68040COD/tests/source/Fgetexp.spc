verbose
#TESTING FGETEXP-- Get exponent
#		-- Converts the source operand to extended precision (if
#		-- necessary) and extracts the binary exponent. Removes the
#		-- exponent bias, converts the exponent to an extended 
#		-- precision floating-point number, and stores the result in
#		-- the destination floating-point data register.


#TEST FGETEXP of +0

fgetexp.d 0000_0000_0000_0000 rn x

#TEST FGETEXP of -0

fgetexp.d 8000_0000_0000_0000 rn x

#TEST FGETEXP of Inf

fgetexp.d 7ff0_0000_0000_0000 rn x

#TEST FGETEXP of -Inf

fgetexp.d fff0_0000_0000_0000 rn x

#TEST FGETEXP of +QNAN

fgetexp.d 7fff_ffff_ffff_ffff rn x

#TEST FGETEXP of -QNAN

fgetexp.d ffff_ffff_ffff_ffff rn x

#TEST FGETEXP of +SNAN

fgetexp.d 7ff7_ffff_ffff_ffff rn x SNAN

#TEST FGETEXP of -SNAN

fgetexp.d fff7_ffff_ffff_ffff rn x SNAN

#TEST FGETEXP of positive Denorm numbers

fgetexp.d 0000_0000_0000_0001 rn x 
fgetexp.d 0008_0000_0000_0000 rn x 
fgetexp.d 0000_0000_8000_0000 rn x 

#TEST FGETEXP of negative Denorm numbers

fgetexp.d 8000_0000_0000_0001 rn x 
fgetexp.d 8008_0000_0000_0000 rn x 
fgetexp.d 8000_0000_8000_0000 rn x 
