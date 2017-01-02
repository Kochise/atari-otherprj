/*
 *		Copyright (C) Motorola, Inc. 1990
 *			All Rights Reserved
 *
 *	For details on the license for this file, please see the
 *	file, README.lic, in this same directory.
 */
#include "template.h"

#include "extdefs.h"

extern exit();

struct template Allops[] = {
#include "body.c"
{T_END, "", exit}
};
