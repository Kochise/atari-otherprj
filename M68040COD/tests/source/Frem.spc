verbose
#TESTING FREM	-- IEEE Remainder
#		-- Converts the source operand to extended precision i(if
#		-- necessary) and calculates the remulo remainder of the number
#		-- in the destination floating-point data register, using the
#		-- the source operand as the remulus.  Stores the result in
#		-- the destination floating-point data register, and stores 
#		-- the sign and seven least significant bits of the quotient
#		-- in the FPSR quotient byte(the quotient is the result of
#		-- FPn(/) Source).  The IEEE remainder function is defined as:
#		-- 	FPn -(Source X N) 
#		-- where:
#		--    	N=INT(FPn(/)Source) in the the round-to-nearest reme
#		-- The FREM function is not defined for a source operand equal
#		-- to zero or for a destination operand equal to infinity. Note
#		-- that this function is not the same as the FREM instruction,
#		-- which uses the round-to-zero reme and thus returns the 
#		-- remainder that is different from the IEEE Specification for
#		-- Binary Floating-Point Arithmetic.


#TEST FREM of +1 rem +inf
frem.d [d]3ff0_0000_0000_0000 7ff0_0000_0000_0000 rn x


#TEST FREM of +1 rem -inf
frem.d [d]3ff0_0000_0000_0000 fff0_0000_0000_0000 rn x


#TEST FREM of -1 rem +inf
frem.d [d]bff0_0000_0000_0000 7ff0_0000_0000_0000 rn x

#TEST FREM of -1 rem -inf
frem.d [d]bff0_0000_0000_0000 fff0_0000_0000_0000 rn x

#TEST FREM of +0 rem +0
frem.d [d]0000_0000_0000_0000 0000_0000_0000_0000 rn x

#TEST FREM of +0 rem -0
frem.d [d]0000_0000_0000_0000 8000_0000_0000_0000 rn x

#TEST FREM of -0 rem +0
frem.d [d]8000_0000_0000_0000 0000_0000_0000_0000 rn x

#TEST FREM of -0 rem -0
frem.d [d]8000_0000_0000_0000 8000_0000_0000_0000 rn x

#TEST FREM of +0 rem +Inf
frem.d [d]0000_0000_0000_0000 7ff0_0000_0000_0000 rn x

#TEST FREM of +0 rem -Inf
frem.d [d]0000_0000_0000_0000 fff0_0000_0000_0000 rn x

#TEST FREM of -0 rem +Inf
frem.d [d]8000_0000_0000_0000 7ff0_0000_0000_0000 rn x

#TEST FREM of -0 rem -Inf
frem.d [d]8000_0000_0000_0000 fff0_0000_0000_0000 rn x

#TEST FREM of +Inf rem +1
frem.d [d]7ff0_0000_0000_0000 3ff0_0000_0000_0000 rn x

#TEST FREM of +Inf rem -1
frem.d [d]7ff0_0000_0000_0000 bff0_0000_0000_0000 rn x

#TEST FREM of -Inf rem +1
frem.d [d]fff0_0000_0000_0000 3ff0_0000_0000_0000 rn x

#TEST FREM of -Inf rem -1
frem.d [d]fff0_0000_0000_0000 bff0_0000_0000_0000 rn x

#TEST FREM of +Inf rem +0
frem.d [d]7ff0_0000_0000_0000 0000_0000_0000_0000 rn x

#TEST FREM of +Inf rem -0
frem.d [d]7ff0_0000_0000_0000 8000_0000_0000_0000 rn x

#TEST FREM of -Inf rem +0
frem.d [d]fff0_0000_0000_0000 0000_0000_0000_0000 rn x

#TEST FREM of -Inf rem -0
frem.d [d]fff0_0000_0000_0000 8000_0000_0000_0000 rn x

#TEST FREM of +Inf rem +Inf
frem.d [d]7ff0_0000_0000_0000 7ff0_0000_0000_0000 rn x

#TEST FREM of +Inf rem -Inf
frem.d [d]7ff0_0000_0000_0000 fff0_0000_0000_0000 rn x

#TEST FREM of -Inf rem +Inf
frem.d [d]fff0_0000_0000_0000 7ff0_0000_0000_0000 rn x

#TEST FREM of -Inf rem -Inf
frem.d [d]fff0_0000_0000_0000 fff0_0000_0000_0000 rn x



#TEST FREM of +0 rem +QNAN
frem.d [d]0000_0000_0000_0000 7fff_ffff_ffff_ffff rn x

#TEST FREM of +0 rem -QNAN
frem.d [d]0000_0000_0000_0000 ffff_ffff_ffff_ffff rn x

#TEST FREM of -0 rem +QNAN
frem.d [d]8000_0000_0000_0000 7fff_ffff_ffff_ffff rn x

#TEST FREM of -0 rem -QNAN
frem.d [d]8000_0000_0000_0000 ffff_ffff_ffff_ffff rn x

#TEST FREM of +QNAN rem +0
frem.d [d]7fff_ffff_ffff_ffff 0000_0000_0000_0000 rn x

#TEST FREM of -QNAN rem +0
frem.d [d]ffff_ffff_ffff_ffff 0000_0000_0000_0000 rn x

#TEST FREM of +QNAN rem -0
frem.d [d]7fff_ffff_ffff_ffff 8000_0000_0000_0000 rn x

#TEST FREM of -QNAN rem -0
frem.d [d]ffff_ffff_ffff_ffff 8000_0000_0000_0000 rn x

#TEST FREM of +SNAN rem +0
frem.d [d]7ff7_ffff_ffff_ffff 0000_0000_0000_0000 rn x SNAN

#TEST FREM of +SNAN rem -0
frem.d [d]7ff7_ffff_ffff_ffff 8000_0000_0000_0000 rn x SNAN

#TEST FREM of -SNAN rem +0
frem.d [d]fff7_ffff_ffff_ffff 0000_0000_0000_0000 rn x SNAN

#TEST FREM of -SNAN rem -0
frem.d [d]fff7_ffff_ffff_ffff 8000_0000_0000_0000 rn x SNAN

#TEST FREM of 1  positive Denorm number rem +0
frem.d [d]0000_0000_0000_0001 0000_0000_0000_0000 rn x

#TEST FREM of 1  negative Denorm number rem -0
frem.d [d]8000_0000_0000_0001 8000_0000_0000_0000 rn x

#TEST FREM of 1  positive Denorm number rem +Inf
frem.d [d]0000_0000_0000_0001 7ff0_0000_0000_0000 rn x

#TEST FREM of 1  negative Denorm number rem -Inf
frem.d [d]8000_0000_0000_0001 fff0_0000_0000_0000 rn x

#TEST FREM of 1  positive Denorm number rem 1 positive Denorm number
frem.d [d]0000_0000_0000_0001 0000_0000_0000_0001 rn x

#TEST FREM of 1  negative Denorm number rem 1 negative Denorm number
frem.d [d]8000_0000_0000_0001 8000_0000_0000_0001 rn x
