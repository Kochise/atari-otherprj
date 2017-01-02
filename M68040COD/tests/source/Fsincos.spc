verbose
#TEST FSINCOS -- Both sine and cosine

#TEST FSINCOS of +0

fsincos.d 0000_0000_0000_0000 rn x

#TEST FSINCOS of -0

fsincos.d 8000_0000_0000_0000 rn x

#TEST FSINCOS of Inf

fsincos.d 7ff0_0000_0000_0000 rn x

#TEST FSINCOS of -Inf

fsincos.d fff0_0000_0000_0000 rn x

#TEST FSINCOS of +QNAN

fsincos.d 7fff_ffff_ffff_ffff rn x

#TEST FSINCOS of -QNAN

fsincos.d ffff_ffff_ffff_ffff rn x

#TEST FSINCOS of +SNAN

fsincos.d 7ff7_ffff_ffff_ffff rn x SNAN

#TEST FSINCOS of -SNAN

fsincos.d fff7_ffff_ffff_ffff rn x SNAN

#TEST FSINCOS of positive Denorm numbers

fsincos.d 0000_0000_0000_0001 rn x 
fsincos.d 0008_0000_0000_0000 rn x 
fsincos.d 0000_0000_8000_0000 rn x 

#TEST FSINCOS of negative Denorm numbers

fsincos.d 8000_0000_0000_0001 rn x 
fsincos.d 8008_0000_0000_0000 rn x 
fsincos.d 8000_0000_8000_0000 rn x 
