verbose
#TESTING FMOVE	-- Move Floating-Point Data Register
#		-- Moves the contents of the source operand to the destination
#		-- destination operand.  Although the primary function of this
#		-- instruction is data movement, it is also considered an 
#		-- arithmetic intruction since conversions from the source
#		-- operand format to the destination format are performed
#		-- implicitly during the move operation.  Also, the source 
#		-- operand is rounded according to the selected rounding
#		-- precision an mode.


#TEST FMOVE of +0

fmove.x 0000_0000_0000_0000_0000_0000 rn x
fmove.x out  0000_0000_0000_0000_0000_0000 rn x

#TEST FMOVE of -0

fmove.x 8000_0000_0000_0000_0000_0000 rn x
fmove.x out 8000_0000_0000_0000_0000_0000 rn x

#TEST FMOVE of Inf

fmove.x 7fff_0000_0000_0000_0000_0000 rn x
fmove.x 7fff_0000_8000_0000_0000_0000 rn x
fmove.x out 7fff_0000_0000_0000_0000_0000 rn x
fmove.x out 7fff_0000_8000_0000_0000_0000 rn x

#TEST FMOVE of -Inf

fmove.x ffff_0000_0000_0000_0000_0000 rn x
fmove.x ffff_0000_8000_0000_0000_0000 rn x
fmove.x out  ffff_0000_0000_0000_0000_0000 rn x
fmove.x out ffff_0000_8000_0000_0000_0000 rn x

#TEST FMOVE of +QNAN

fmove.x 7fff_0000_7fff_ffff_ffff_ffff rn x
fmove.x 7fff_0000_ffff_ffff_ffff_ffff rn x
fmove.x out 7fff_0000_7fff_ffff_ffff_ffff rn x
fmove.x out 7fff_0000_ffff_ffff_ffff_ffff rn x

#TEST FMOVE of -QNAN

fmove.x ffff_0000_7fff_ffff_ffff_ffff  rn x
fmove.x ffff_0000_ffff_ffff_ffff_ffff  rn x
fmove.x out ffff_0000_7fff_ffff_ffff_ffff  rn x
fmove.x out ffff_0000_ffff_ffff_ffff_ffff  rn x

#TEST FMOVE of +SNAN

fmove.x 7fff_0000_3fff_ffff_ffff_ffff rn x SNAN
fmove.x 7fff_0000_bfff_ffff_ffff_ffff rn x SNAN
fmove.x out 7fff_0000_3fff_ffff_ffff_ffff rn x SNAN
fmove.x out 7fff_0000_bfff_ffff_ffff_ffff rn x SNAN

#TEST FMOVE of -SNAN

fmove.x ffff_0000_3fff_ffff_ffff_ffff rn x SNAN
fmove.x ffff_0000_bfff_ffff_ffff_ffff rn x SNAN
fmove.x out ffff_0000_3fff_ffff_ffff_ffff rn x SNAN
fmove.x out ffff_0000_bfff_ffff_ffff_ffff rn x SNAN

#TEST FMOVE of 1  positive Denorm number

fmove.x 0000_0000_0000_0000_0000_0001 rn x 

#TEST FMOVE of 1  negative Denorm number

fmove.x 8000_0000_0000_0000_0000_0001 rn x 

#======================================================================
#PACKED MOVE

#TEST FMOVE.p of +inrange number
fmove.p 0999_9999_9999_9999_9999_9999
fmove.p 4999_9999_9999_9999_9999_9999

fmove.p out 3fff_0000_8010_0101_0101_0101 k#:-64
fmove.p out 3fff_0000_8010_0101_0101_0101 k#:-63
fmove.p out 3fff_0000_8010_0101_0101_0101 k#:17
fmove.p out 3fff_0000_8010_0101_0101_0101 k#:18
fmove.p out 3fff_0000_8010_0101_0101_0101 k#:-5
fmove.p out 3fff_0000_8010_0101_0101_0101 k#:-3
fmove.p out 3fff_0000_8010_0101_0101_0101 k#:-1
fmove.p out 3fff_0000_8010_0101_0101_0101 k#:0
fmove.p out 3fff_0000_8010_0101_0101_0101 k#:1
fmove.p out 3fff_0000_8010_0101_0101_0101 k#:3
fmove.p out 3fff_0000_8010_0101_0101_0101 k#:5


#TEST FMOVE.p of -inrange number
fmove.p 8999_9999_9999_9999_9999_9999
fmove.p c999_9999_9999_9999_9999_9999

fmove.p out c000_0000_c010_0101_0101_0101 k#:-64
fmove.p out c000_0000_c010_0101_0101_0101 k#:-63
fmove.p out c000_0000_c010_0101_0101_0101 k#:17
fmove.p out c000_0000_c010_0101_0101_0101 k#:18
fmove.p out c000_0000_c010_0101_0101_0101 k#:-5
fmove.p out c000_0000_c010_0101_0101_0101 k#:-3
fmove.p out c000_0000_c010_0101_0101_0101 k#:-1
fmove.p out c000_0000_c010_0101_0101_0101 k#:0
fmove.p out c000_0000_c010_0101_0101_0101 k#:1
fmove.p out c000_0000_c010_0101_0101_0101 k#:3
fmove.p out c000_0000_c010_0101_0101_0101 k#:5

#TEST FMOVE.p of +0
fmove.p 0999_9990_0000_0000_0000_0000
fmove.p 4999_9990_0000_0000_0000_0000

fmove.p out 0000_0000_0000_0000_0000_0000 k#:-64
fmove.p out 0000_0000_0000_0000_0000_0000 k#:-63
fmove.p out 0000_0000_0000_0000_0000_0000 k#:17
fmove.p out 0000_0000_0000_0000_0000_0000 k#:18
fmove.p out 0000_0000_0000_0000_0000_0000 k#:-5
fmove.p out 0000_0000_0000_0000_0000_0000 k#:-3
fmove.p out 0000_0000_0000_0000_0000_0000 k#:-1
fmove.p out 0000_0000_0000_0000_0000_0000 k#:0
fmove.p out 0000_0000_0000_0000_0000_0000 k#:1
fmove.p out 0000_0000_0000_0000_0000_0000 k#:3
fmove.p out 0000_0000_0000_0000_0000_0000 k#:5

#TEST FMOVE.p of -0
fmove.p 8999_9990_0000_0000_0000_0000
fmove.p c999_9990_0000_0000_0000_0000

fmove.p out 8000_0000_0000_0000_0000_0000 k#:-64
fmove.p out 8000_0000_0000_0000_0000_0000 k#:-63
fmove.p out 8000_0000_0000_0000_0000_0000 k#:17
fmove.p out 8000_0000_0000_0000_0000_0000 k#:18
fmove.p out 8000_0000_0000_0000_0000_0000 k#:-5
fmove.p out 8000_0000_0000_0000_0000_0000 k#:-3
fmove.p out 8000_0000_0000_0000_0000_0000 k#:-1
fmove.p out 8000_0000_0000_0000_0000_0000 k#:0
fmove.p out 8000_0000_0000_0000_0000_0000 k#:1
fmove.p out 8000_0000_0000_0000_0000_0000 k#:3
fmove.p out 8000_0000_0000_0000_0000_0000 k#:5

#TEST FMOVE.p of +Inf
fmove.p 7fff_0000_0000_0000_0000_0000

fmove.p out 7fff_0000_0000_0000_0000_0000 k#:-64
fmove.p out 7fff_0000_0000_0000_0000_0000 k#:-63
fmove.p out 7fff_0000_0000_0000_0000_0000 k#:17
fmove.p out 7fff_0000_0000_0000_0000_0000 k#:18
fmove.p out 7fff_0000_0000_0000_0000_0000 k#:-5
fmove.p out 7fff_0000_0000_0000_0000_0000 k#:-3
fmove.p out 7fff_0000_0000_0000_0000_0000 k#:-1
fmove.p out 7fff_0000_0000_0000_0000_0000 k#:0
fmove.p out 7fff_0000_0000_0000_0000_0000 k#:1
fmove.p out 7fff_0000_0000_0000_0000_0000 k#:3
fmove.p out 7fff_0000_0000_0000_0000_0000 k#:5

#TEST FMOVE.p of -Inf
fmove.p ffff_0000_0000_0000_0000_0000

fmove.p out ffff_0000_0000_0000_0000_0000 k#:-64
fmove.p out ffff_0000_0000_0000_0000_0000 k#:-63
fmove.p out ffff_0000_0000_0000_0000_0000 k#:17
fmove.p out ffff_0000_0000_0000_0000_0000 k#:18
fmove.p out ffff_0000_0000_0000_0000_0000 k#:-5
fmove.p out ffff_0000_0000_0000_0000_0000 k#:-3
fmove.p out ffff_0000_0000_0000_0000_0000 k#:-1
fmove.p out ffff_0000_0000_0000_0000_0000 k#:0
fmove.p out ffff_0000_0000_0000_0000_0000 k#:1
fmove.p out ffff_0000_0000_0000_0000_0000 k#:3
fmove.p out ffff_0000_0000_0000_0000_0000 k#:5

#TEST FMOVE.p of +NAN
fmove.p 7fff_0000_4000_0000_0000_0001

fmove.p out 7fff_0000_ffff_ffff_ffff_ffff k#:-64
fmove.p out 7fff_0000_ffff_ffff_ffff_ffff k#:-63
fmove.p out 7fff_0000_ffff_ffff_ffff_ffff k#:17
fmove.p out 7fff_0000_ffff_ffff_ffff_ffff k#:18
fmove.p out 7fff_0000_ffff_ffff_ffff_ffff k#:-5
fmove.p out 7fff_0000_ffff_ffff_ffff_ffff k#:-3
fmove.p out 7fff_0000_ffff_ffff_ffff_ffff k#:-1
fmove.p out 7fff_0000_ffff_ffff_ffff_ffff k#:0
fmove.p out 7fff_0000_ffff_ffff_ffff_ffff k#:1
fmove.p out 7fff_0000_ffff_ffff_ffff_ffff k#:3
fmove.p out 7fff_0000_ffff_ffff_ffff_ffff k#:5

#TEST FMOVE.p of -NAN
fmove.p ffff_0000_4000_0000_0000_0001

fmove.p out ffff_0000_ffff_ffff_ffff_ffff k#:-64
fmove.p out ffff_0000_ffff_ffff_ffff_ffff k#:-63
fmove.p out ffff_0000_ffff_ffff_ffff_ffff k#:17
fmove.p out ffff_0000_ffff_ffff_ffff_ffff k#:18
fmove.p out ffff_0000_ffff_ffff_ffff_ffff k#:-5
fmove.p out ffff_0000_ffff_ffff_ffff_ffff k#:-3
fmove.p out ffff_0000_ffff_ffff_ffff_ffff k#:-1
fmove.p out ffff_0000_ffff_ffff_ffff_ffff k#:0
fmove.p out ffff_0000_ffff_ffff_ffff_ffff k#:1
fmove.p out ffff_0000_ffff_ffff_ffff_ffff k#:3
fmove.p out ffff_0000_ffff_ffff_ffff_ffff k#:5

#TEST FMOVE.p of +SNAN
fmove.p 7fff_0000_0000_0000_0000_0001

fmove.p out 7fff_0000_bfff_ffff_ffff_ffff k#:-64
fmove.p out 7fff_0000_bfff_ffff_ffff_ffff k#:-63
fmove.p out 7fff_0000_bfff_ffff_ffff_ffff k#:17
fmove.p out 7fff_0000_bfff_ffff_ffff_ffff k#:18
fmove.p out 7fff_0000_bfff_ffff_ffff_ffff k#:-5
fmove.p out 7fff_0000_bfff_ffff_ffff_ffff k#:-3
fmove.p out 7fff_0000_bfff_ffff_ffff_ffff k#:-1
fmove.p out 7fff_0000_bfff_ffff_ffff_ffff k#:0
fmove.p out 7fff_0000_bfff_ffff_ffff_ffff k#:1
fmove.p out 7fff_0000_bfff_ffff_ffff_ffff k#:3
fmove.p out 7fff_0000_bfff_ffff_ffff_ffff k#:5

#TEST FMOVE.p of -SNAN
fmove.p ffff_0000_0000_0000_0000_0001

fmove.p out ffff_0000_bfff_ffff_ffff_ffff k#:-64
fmove.p out ffff_0000_bfff_ffff_ffff_ffff k#:-63
fmove.p out ffff_0000_bfff_ffff_ffff_ffff k#:17
fmove.p out ffff_0000_bfff_ffff_ffff_ffff k#:18
fmove.p out ffff_0000_bfff_ffff_ffff_ffff k#:-5
fmove.p out ffff_0000_bfff_ffff_ffff_ffff k#:-3
fmove.p out ffff_0000_bfff_ffff_ffff_ffff k#:-1
fmove.p out ffff_0000_bfff_ffff_ffff_ffff k#:0
fmove.p out ffff_0000_bfff_ffff_ffff_ffff k#:1
fmove.p out ffff_0000_bfff_ffff_ffff_ffff k#:3
fmove.p out ffff_0000_bfff_ffff_ffff_ffff k#:5
