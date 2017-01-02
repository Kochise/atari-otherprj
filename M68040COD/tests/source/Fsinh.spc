verbose
#TESTING FSINH	-- Hyperbolic Sine of Source
#		-- Converts the source operand to extended precision ( if
#		-- necessary) and calculates the hyperbolic cosinhe of that
#		-- number.  Stores the result in the destination floating-point
#		-- data register.

#TEST FSINH of +0

fsinh.d 0000_0000_0000_0000 rn x

#TEST FSINH of -0

fsinh.d 8000_0000_0000_0000 rn x

#TEST FSINH of Inf

fsinh.d 7ff0_0000_0000_0000 rn x

#TEST FSINH of -Inf

fsinh.d fff0_0000_0000_0000 rn x

#TEST FSINH of +QNAN

fsinh.d 7fff_ffff_ffff_ffff rn x

#TEST FSINH of -QNAN

fsinh.d ffff_ffff_ffff_ffff rn x

#TEST FSINH of +SNAN

fsinh.d 7ff7_ffff_ffff_ffff rn x SNAN

#TEST FSINH of -SNAN

fsinh.d fff7_ffff_ffff_ffff rn x SNAN

#TEST FSINH of positive Denorm numbers

fsinh.d 0000_0000_0000_0001 rn x 
fsinh.d 0008_0000_0000_0000 rn x 
fsinh.d 0000_0000_8000_0000 rn x 

#TEST FSINH of negative Denorm numbers

fsinh.d 8000_0000_0000_0001 rn x 
fsinh.d 8008_0000_0000_0000 rn x 
fsinh.d 8000_0000_8000_0000 rn x 
