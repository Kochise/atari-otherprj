verbose
#TESTING FSCALE	-- Scale Exponent
#		-- Converts the source operand to an integer(if necessary) and
#		-- ad s that integer to the destination exponent. Stores the
#		-- result in the  estination floating-point data register.
#		-- This function has the effect of multiplying the  estination
#		-- by 2 to the Source power, but is much faster than a multiply
#		-- operation when the source is an integer value.
#		-- The FPCP assumes that the scale factore is an integer value
#		-- before the operation is execute .  If not, the value is
#		-- choppe (i.e. roundeddusing the round-to-zero mode) to an
#		-- integer before it is ad e[d]to the exponent.  When the 
#		-- absolute value of the source operand is (>=)2 to the 14th, 
#		-- and overflow or underflow always results.


#TEST FSCALE of +1 scale +inf
fscale.d  [d]3ff0_0000_0000_0000 7ff0_0000_0000_0000 rn x


#TEST FSCALE of +1 scale -inf
fscale.d  [d]3ff0_0000_0000_0000 fff0_0000_0000_0000 rn x


#TEST FSCALE of -1 scale +inf
fscale.d [d]bff0_0000_0000_0000 7ff0_0000_0000_0000 rn x

#TEST FSCALE of -1 scale -inf
fscale.d [d]bff0_0000_0000_0000 fff0_0000_0000_0000 rn x

#TEST FSCALE of +0 scale +0
fscale.d [d]0000_0000_0000_0000 0000_0000_0000_0000 rn x

#TEST FSCALE of +0 scale -0
fscale.d [d]0000_0000_0000_0000 8000_0000_0000_0000 rn x

#TEST FSCALE of -0 scale +0
fscale.d [d]8000_0000_0000_0000 0000_0000_0000_0000 rn x

#TEST FSCALE of -0 scale -0
fscale.d [d]8000_0000_0000_0000 8000_0000_0000_0000 rn x

#TEST FSCALE of +0 scale +Inf
fscale.d [d]0000_0000_0000_0000 7ff0_0000_0000_0000 rn x

#TEST FSCALE of +0 scale -Inf
fscale.d [d]0000_0000_0000_0000 fff0_0000_0000_0000 rn x

#TEST FSCALE of -0 scale +Inf
fscale.d [d]8000_0000_0000_0000 7ff0_0000_0000_0000 rn x

#TEST FSCALE of -0 scale -Inf
fscale.d [d]8000_0000_0000_0000 fff0_0000_0000_0000 rn x

#TEST FSCALE of +Inf scale +1
fscale.d [d]7ff0_0000_0000_0000 3ff0_0000_0000_0000 rn x

#TEST FSCALE of +Inf scale -1
fscale.d [d]7ff0_0000_0000_0000 bff0_0000_0000_0000 rn x

#TEST FSCALE of -Inf scale +1
fscale.d [d]fff0_0000_0000_0000 3ff0_0000_0000_0000 rn x

#TEST FSCALE of -Inf scale -1
fscale.d [d]fff0_0000_0000_0000 bff0_0000_0000_0000 rn x

#TEST FSCALE of +Inf scale +0
fscale.d [d]7ff0_0000_0000_0000 0000_0000_0000_0000 rn x

#TEST FSCALE of +Inf scale -0
fscale.d [d]7ff0_0000_0000_0000 8000_0000_0000_0000 rn x

#TEST FSCALE of -Inf scale +0
fscale.d [d]fff0_0000_0000_0000 0000_0000_0000_0000 rn x

#TEST FSCALE of -Inf scale -0
fscale.d [d]fff0_0000_0000_0000 8000_0000_0000_0000 rn x

#TEST FSCALE of +Inf scale +Inf
fscale.d [d]7ff0_0000_0000_0000 7ff0_0000_0000_0000 rn x

#TEST FSCALE of +Inf scale -Inf
fscale.d [d]7ff0_0000_0000_0000 fff0_0000_0000_0000 rn x

#TEST FSCALE of -Inf scale +Inf
fscale.d [d]fff0_0000_0000_0000 7ff0_0000_0000_0000 rn x

#TEST FSCALE of -Inf scale -Inf
fscale.d [d]fff0_0000_0000_0000 fff0_0000_0000_0000 rn x



#TEST FSCALE of +0 scale +QNAN
fscale.d [d]0000_0000_0000_0000 7fff_ffff_ffff_ffff rn x

#TEST FSCALE of +0 scale -QNAN
fscale.d [d]0000_0000_0000_0000 ffff_ffff_ffff_ffff rn x

#TEST FSCALE of -0 scale +QNAN
fscale.d [d]8000_0000_0000_0000 7fff_ffff_ffff_ffff rn x

#TEST FSCALE of -0 scale -QNAN
fscale.d [d]8000_0000_0000_0000 ffff_ffff_ffff_ffff rn x

#TEST FSCALE of +QNAN scale +0
fscale.d [d]7fff_ffff_ffff_ffff 0000_0000_0000_0000 rn x

#TEST FSCALE of -QNAN scale +0
fscale.d [d]ffff_ffff_ffff_ffff 0000_0000_0000_0000 rn x

#TEST FSCALE of +QNAN scale -0
fscale.d [d]7fff_ffff_ffff_ffff 8000_0000_0000_0000 rn x

#TEST FSCALE of -QNAN scale -0
fscale.d [d]ffff_ffff_ffff_ffff 8000_0000_0000_0000 rn x

#TEST FSCALE of +SNAN scale +0
fscale.d [d]7ff7_ffff_ffff_ffff 0000_0000_0000_0000 rn x SNAN

#TEST FSCALE of +SNAN scale -0
fscale.d [d]7ff7_ffff_ffff_ffff 8000_0000_0000_0000 rn x SNAN

#TEST FSCALE of -SNAN scale +0
fscale.d [d]fff7_ffff_ffff_ffff 0000_0000_0000_0000 rn x SNAN

#TEST FSCALE of -SNAN scale -0
fscale.d [d]fff7_ffff_ffff_ffff 8000_0000_0000_0000 rn x SNAN

#TEST FSCALE of 1  positive Denorm number scale +0
fscale.d [d]0000_0000_0000_0001 0000_0000_0000_0000 rn x

#TEST FSCALE of 1  negative Denorm number scale -0
fscale.d [d]8000_0000_0000_0001 8000_0000_0000_0000 rn x

#TEST FSCALE of 1  positive Denorm number scale +Inf
fscale.d [d]0000_0000_0000_0001 7ff0_0000_0000_0000 rn x

#TEST FSCALE of 1  negative Denorm number scale -Inf
fscale.d [d]8000_0000_0000_0001 fff0_0000_0000_0000 rn x

#TEST FSCALE of 1  positive Denorm number scale 1 positive Denorm number
fscale.d [d]0000_0000_0000_0001 0000_0000_0000_0001 rn x

#TEST FSCALE of 1  negative Denorm number scale 1 negative Denorm number
fscale.d [d]8000_0000_0000_0001 8000_0000_0000_0001 rn x
