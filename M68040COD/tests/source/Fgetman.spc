verbose
#TESTING FGETMAN-- Get Mantissa
#		-- Converts the source operand to extended precision (if
#		-- necessary) and extracts the mantissa.  Converts the mantissa
#		-- to an extended precision value and stores the result in the
#		-- destination floating-point data register.  The result is in
#		-- the range [1.0...2.0) with the sign of the source mantissa,
#		-- zero, or is a NAN. 


#TEST FGETMAN of +0

fgetman.d 0000_0000_0000_0000 rn x

#TEST FGETMAN of -0

fgetman.d 8000_0000_0000_0000 rn x

#TEST FGETMAN of Inf

fgetman.d 7ff0_0000_0000_0000 rn x

#TEST FGETMAN of -Inf

fgetman.d fff0_0000_0000_0000 rn x

#TEST FGETMAN of +QNAN

fgetman.d 7fff_ffff_ffff_ffff rn x

#TEST FGETMAN of -QNAN

fgetman.d ffff_ffff_ffff_ffff rn x

#TEST FGETMAN of +SNAN

fgetman.d 7ff7_ffff_ffff_ffff rn x SNAN

#TEST FGETMAN of -SNAN

fgetman.d fff7_ffff_ffff_ffff rn x SNAN

#TEST FGETMAN of positive Denorm numbers

fgetman.d 0000_0000_0000_0001 rn x 
fgetman.d 0008_0000_0000_0000 rn x 
fgetman.d 0000_0000_8000_0000 rn x 

#TEST FGETMAN of negative Denorm numbers

fgetman.d 8000_0000_0000_0001 rn x 
fgetman.d 8008_0000_0000_0000 rn x 
fgetman.d 8000_0000_8000_0000 rn x 
