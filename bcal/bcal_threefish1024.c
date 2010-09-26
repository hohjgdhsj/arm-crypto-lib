/* bcal_threefish1024.c */
/*
    This file is part of the ARM-Crypto-Lib.
    Copyright (C) 2006-2010  Daniel Otte (daniel.otte@rub.de)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * \file     bcal_threefish1024.c
 * \email    daniel.otte@rub.de
 * \author   Daniel Otte
 * \date     2010-02-20
 * \license  GPLv3 or later
 *
 */

#include <avr/pgmspace.h>
#include <stdlib.h>
#include "blockcipher_descriptor.h"
#include "threefish.h"
#include "keysize_descriptor.h"

const char threefish1024_str[]   PROGMEM = "Threefish-1024";

const uint8_t threefish1024_keysize_desc[] PROGMEM = { KS_TYPE_LIST, 1, KS_INT(1024),
                                                KS_TYPE_TERMINATOR    };

static void threefish1024_dummy_init(void* key, void* ctx){
	threefish1024_init(key, NULL, ctx);
}

const bcdesc_t threefish1024_desc PROGMEM = {
	BCDESC_TYPE_BLOCKCIPHER,
	BC_INIT_TYPE_1,
	threefish1024_str,
	sizeof(threefish1024_ctx_t),
	1024,
	{(void_fpt)threefish1024_dummy_init},
	{(void_fpt)threefish1024_enc},
	{(void_fpt)threefish1024_dec},
	(bc_free_fpt)NULL,
	threefish1024_keysize_desc
};


