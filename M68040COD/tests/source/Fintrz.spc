verbose
#TESTING FINTRZ  -- Integer Part,Rounded-to-Zero
#		-- Converts the source operand to extended precision (if
#		-- necessary) and extracts the intrzeger part and converts it to
#		-- an extended precision floating-pointrz number.  Stores the
#		-- result in the destination floating-pointrz data register.
#		-- The intrzeger part is extracted by rounding the extended
#		-- precision number to an intrzeger using the round-to-zero
#		-- mode regardless of the rounding mode selected in the 
#		-- FPCR mode control byte.  


#TEST FINTRZ of +0

fintrz.d 0000_0000_0000_0000 rn x

#TEST FINTRZ of -0

fintrz.d 8000_0000_0000_0000 rn x

#TEST FINTRZ of Inf

fintrz.d 7ff0_0000_0000_0000 rn x

#TEST FINTRZ of -Inf

fintrz.d fff0_0000_0000_0000 rn x

#TEST FINTRZ of +QNAN

fintrz.d 7fff_ffff_ffff_ffff rn x

#TEST FINTRZ of -QNAN

fintrz.d ffff_ffff_ffff_ffff rn x

#TEST FINTRZ of +SNAN

fintrz.d 7ff7_ffff_ffff_ffff rn x SNAN

#TEST FINTRZ of -SNAN

fintrz.d fff7_ffff_ffff_ffff rn x SNAN

#TEST FINTRZ of positive Denorm numbers

fintrz.d 0000_0000_0000_0001 rn x 
fintrz.d 0008_0000_0000_0000 rn x 
fintrz.d 0000_0000_8000_0000 rn x 

#TEST FINTRZ of negative Denorm numbers

fintrz.d 8000_0000_0000_0001 rn x 
fintrz.d 8008_0000_0000_0000 rn x 
fintrz.d 8000_0000_8000_0000 rn x 
