
#	L_ENTRY.AWK 1.1 3/27/91

#		Copyright (C) Motorola, Inc. 1991
#			All Rights Reserved
#
#	For details on the license for this file, please see the
#	file, README, in this same directory.

BEGIN{
	print "echo \"	section 8\""
	print "echo \"	include	l_fpsp.h\""
	print "echo \"\""
	print "echo \"	xref	tag\""
	print "echo \"	xref	szero\""
	print "echo \"	xref	sinf\""
	print "echo \"	xref	sopr_inf\""
	print "echo \"	xref	sone\""
	print "echo \"	xref	spi_2\""
	print "echo \"	xref	szr_inf\""
	print "echo \"	xref	src_nan\""
	print "echo \"	xref	t_operr\""
	print "echo \"	xref	t_dz2\""
	print "echo \"	xref	snzrinx\""
	print "echo \"	xref	ld_pone\""
	print "echo \"	xref	ld_pinf\""
	print "echo \"	xref	ld_ppi2\""
	print "echo \"	xref	ssincosz\""
	print "echo \"	xref	ssincosi\""
	print "echo \"	xref	ssincosnan\""
	print "echo \"	xref	setoxm1i\""
	utmp = 100
	}

$4=="MONADIC"{
	printf "sed 's/_OPs_/" $1 "/g' MONADIC." SYS " | "
	printf "sed 's/_OPd_/" $2 "/g' | "
	printf "sed 's/_OPx_/" $3 "/g' | "
	printf "sed 's/_OPr_/" $5 "/g' | "
	printf "sed 's/_OPz_/" $6 "/g' | "
	printf "sed 's/_OPi_/" $7 "/g' | "
	printf "sed 's/_OPn_/" $8 "/g' | "
	printf "sed 's/_OPm_/" $9 "/g' | "
	utmp += 1
	printf "sed 's/_TMP_/" PREFIX utmp "/g'\n "
	}

$4=="DYADIC"{
	printf "sed 's/_OPs_/" $1 "/g' DYADIC." SYS " | "
	printf "sed 's/_OPd_/" $2 "/g' | "
	printf "sed 's/_OPx_/" $3 "/g' | "
	printf "sed 's/_OPa_/" $5 "/g' | "
	utmp += 1
	printf "sed 's/_TMP_/" PREFIX utmp "/g'\n"
	}

END{
	print "echo \"	end\""
	}
