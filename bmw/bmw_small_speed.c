/* bmw_small.c */
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
/*
 * \file    bmw_small.c
 * \author  Daniel Otte
 * \email   daniel.otte@rub.de
 * \date    2009-04-27
 * \license GPLv3 or later
 *
 */

#include <stdint.h>
#include <string.h>
#include "bmw_small.h"
#include "memxor.h"

#define SHL32(a,n) ((a)<<(n))
#define SHR32(a,n) ((a)>>(n))
#define ROTL32(a,n) (((a)<<(n))|((a)>>(32-(n))))
#define ROTR32(a,n) (((a)>>(n))|((a)<<(32-(n))))


#define DEBUG 0


#if DEBUG
 #include "cli.h"

 void ctx_dump(const bmw_small_ctx_t* ctx){
 	uint8_t i;
	cli_putstr("\r\n==== ctx dump ====");
	for(i=0; i<16;++i){
		cli_putstr("\r\n h[");
		cli_hexdump(&i, 1);
		cli_putstr("] = ");
		cli_hexdump_rev(&(ctx->h[i]), 4);
	}
	cli_putstr("\r\n counter = ");
	cli_hexdump(&(ctx->counter), 4);
 }

 void dump_x(const uint32_t* q, uint8_t elements, char x){
	uint8_t i;
 	cli_putstr("\r\n==== ");
	cli_putc(x);
	cli_putstr(" dump ====");
	for(i=0; i<elements;++i){
		cli_putstr("\r\n ");
		cli_putc(x);
		cli_putstr("[");
		cli_hexdump(&i, 1);
		cli_putstr("] = ");
		cli_hexdump_rev(&(q[i]), 4);
	}
 }
#else
 #define ctx_dump(x)
 #define dump_x(a,b,c)
#endif

#define S32_0(x) ( (SHR32((x),   1)) ^ \
	               (SHL32((x),   3)) ^ \
	               (ROTL32((x),  4)) ^ \
	               (ROTR32((x), 13)) )

#define S32_1(x) ( (SHR32((x),   1)) ^ \
	               (SHL32((x),   2)) ^ \
	               (ROTL32((x),  8)) ^ \
	               (ROTR32((x),  9)) )

#define S32_2(x) ( (SHR32((x),   2)) ^ \
	               (SHL32((x),   1)) ^ \
	               (ROTL32((x), 12)) ^ \
	               (ROTR32((x),  7)) )

#define S32_3(x) ( (SHR32((x),   2)) ^ \
	               (SHL32((x),   2)) ^ \
	               (ROTL32((x), 15)) ^ \
	               (ROTR32((x),  3)) )

#define S32_4(x) ( (SHR32((x),   1)) ^ (x))

#define S32_5(x) ( (SHR32((x),   2)) ^ (x))

#define R32_1(x)   (ROTL32((x),  3))
#define R32_2(x)   (ROTL32((x),  7))
#define R32_3(x)   (ROTL32((x), 13))
#define R32_4(x)   (ROTL32((x), 16))
#define R32_5(x)   (ROTR32((x), 13))
#define R32_6(x)   (ROTR32((x),  9))
#define R32_7(x)   (ROTR32((x),  5))

/*
#define K 0x05555555L
static
uint32_t k_lut[] PROGMEM = {
	16L*K, 17L*K, 18L*K, 19L*K, 20L*K, 21L*K, 22L*K, 23L*K,
	24L*K, 25L*K, 26L*K, 27L*K, 28L*K, 29L*K, 30L*K, 31L*K
};
*/
/* same as above but precomputed to avoid compiler warnings */
/*
static
uint32_t k_lut[] = {
	0x55555550L, 0x5aaaaaa5L, 0x5ffffffaL,
	0x6555554fL, 0x6aaaaaa4L, 0x6ffffff9L,
	0x7555554eL, 0x7aaaaaa3L, 0x7ffffff8L,
	0x8555554dL, 0x8aaaaaa2L, 0x8ffffff7L,
	0x9555554cL, 0x9aaaaaa1L, 0x9ffffff6L,
	0xa555554bL };
*/
/*
static inline
uint32_t bmw_small_expand1(uint8_t j, const uint32_t* q, const void* m, const void* h){
	uint32_t r;
	r = (   ROTL32(((uint32_t*)m)[j],      ((j+0))+1  )
	       + ROTL32(((uint32_t*)m)[(j+3)],  ((j+3))+1  )
	       - ROTL32(((uint32_t*)m)[(j+10)], ((j+10))+1 )
	       + k_lut[j]
	     ) ^ ((uint32_t*)h)[(j+7)];
	r += S32_1(q[j+ 0]) + S32_2(q[j+ 1]) + S32_3(q[j+ 2]) + S32_0(q[j+ 3]) +
		 S32_1(q[j+ 4]) + S32_2(q[j+ 5]) + S32_3(q[j+ 6]) + S32_0(q[j+ 7]) +
		 S32_1(q[j+ 8]) + S32_2(q[j+ 9]) + S32_3(q[j+10]) + S32_0(q[j+11]) +
		 S32_1(q[j+12]) + S32_2(q[j+13]) + S32_3(q[j+14]) + S32_0(q[j+15]);

	return r;
}

static inline
uint32_t bmw_small_expand2(uint8_t j, const uint32_t* q, const void* m, const void* h){
	uint32_t r;
	r = (   ROTL32(((uint32_t*)m)[j&0xf],      ((j+0)&0xf)+1  )
	       + ROTL32(((uint32_t*)m)[(j+3)&0xf],  ((j+3)&0xf)+1  )
	       - ROTL32(((uint32_t*)m)[(j+10)&0xf], ((j+10)&0xf)+1 )
	       + k_lut[j]
	     ) ^ ((uint32_t*)h)[(j+7)&0xf];
	r += (q[j+ 0]) + R32_1(q[j+ 1]) + (q[j+ 2]) + R32_2(q[j+ 3]) +
		 (q[j+ 4]) + R32_3(q[j+ 5]) + (q[j+ 6]) + R32_4(q[j+ 7]) +
		 (q[j+ 8]) + R32_5(q[j+ 9]) + (q[j+10]) + R32_6(q[j+11]) +
		 (q[j+12]) + R32_7(q[j+13]) + S32_4(q[j+14]) + S32_5(q[j+15]);
	return r;
}
*/
static inline
void bmw_small_f1(uint32_t* q, const void* m, const void* h);

static inline
void bmw_small_f0(uint32_t* q, uint32_t* h, const uint32_t* m){
	h[ 0] ^= m[ 0];
	h[ 1] ^= m[ 1];
	h[ 2] ^= m[ 2];
	h[ 3] ^= m[ 3];
	h[ 4] ^= m[ 4];
	h[ 5] ^= m[ 5];
	h[ 6] ^= m[ 6];
	h[ 7] ^= m[ 7];
	h[ 8] ^= m[ 8];
	h[ 9] ^= m[ 9];
	h[10] ^= m[10];
	h[11] ^= m[11];
	h[12] ^= m[12];
	h[13] ^= m[13];
	h[14] ^= m[14];
	h[15] ^= m[15];

	dump_x(h, 16, 'T');
	q[ 0] = (h[ 5] - h[ 7] + h[10] + h[13] + h[14]);
	q[ 3] = (h[ 0] - h[ 1] + h[ 8] - h[10] + h[13]);
	q[ 6] = (h[ 4] - h[ 0] - h[ 3] - h[11] + h[13]);
	q[ 9] = (h[ 0] - h[ 3] + h[ 6] - h[ 7] + h[14]);
	q[12] = (h[ 1] + h[ 3] - h[ 6] - h[ 9] + h[10]);
	q[15] = (h[12] - h[ 4] - h[ 6] - h[ 9] + h[13]);
	q[ 2] = (h[ 0] + h[ 7] + h[ 9] - h[12] + h[15]);
	q[ 5] = (h[ 3] - h[ 2] + h[10] - h[12] + h[15]);
	q[ 8] = (h[ 2] - h[ 5] - h[ 6] + h[13] - h[15]);
	q[11] = (h[ 8] - h[ 0] - h[ 2] - h[ 5] + h[ 9]);
	q[14] = (h[ 3] - h[ 5] + h[ 8] - h[11] - h[12]);
	q[ 1] = (h[ 6] - h[ 8] + h[11] + h[14] - h[15]);
	q[ 4] = (h[ 1] + h[ 2] + h[ 9] - h[11] - h[14]);
	q[ 7] = (h[ 1] - h[ 4] - h[ 5] - h[12] - h[14]);
	q[10] = (h[ 8] - h[ 1] - h[ 4] - h[ 7] + h[15]);
	q[13] = (h[ 2] + h[ 4] + h[ 7] + h[10] + h[11]);
	dump_x(q, 16, 'W');
/*
	q[ 0] = S32_0(q[ 0]); q[ 1] = S32_1(q[ 1]); q[ 2] = S32_2(q[ 2]); q[ 3] = S32_3(q[ 3]); q[ 4] = S32_4(q[ 4]);
	q[ 5] = S32_0(q[ 5]); q[ 6] = S32_1(q[ 6]); q[ 7] = S32_2(q[ 7]); q[ 8] = S32_3(q[ 8]); q[ 9] = S32_4(q[ 9]);
	q[10] = S32_0(q[10]); q[11] = S32_1(q[11]); q[12] = S32_2(q[12]); q[13] = S32_3(q[13]); q[14] = S32_4(q[14]);
	q[15] = S32_0(q[15]);
	q[ 0] += h[ 1] ^= m[ 1];
	q[ 1] += h[ 2] ^= m[ 2];
	q[ 2] += h[ 3] ^= m[ 3];
	q[ 3] += h[ 4] ^= m[ 4];
	q[ 4] += h[ 5] ^= m[ 5];
	q[ 5] += h[ 6] ^= m[ 6];
	q[ 6] += h[ 7] ^= m[ 7];
	q[ 7] += h[ 8] ^= m[ 8];
	q[ 8] += h[ 9] ^= m[ 9];
	q[ 9] += h[10] ^= m[10];
	q[10] += h[11] ^= m[11];
	q[11] += h[12] ^= m[12];
	q[12] += h[13] ^= m[13];
	q[13] += h[14] ^= m[14];
	q[14] += h[15] ^= m[15];
	q[15] += h[ 0] ^= m[ 0];
*/
	q[ 0] = S32_0(q[ 0]) + (h[ 1] ^= m[ 1]);
	q[ 1] = S32_1(q[ 1]) + (h[ 2] ^= m[ 2]);
	q[ 2] = S32_2(q[ 2]) + (h[ 3] ^= m[ 3]);
	q[ 3] = S32_3(q[ 3]) + (h[ 4] ^= m[ 4]);
	q[ 4] = S32_4(q[ 4]) + (h[ 5] ^= m[ 5]);
	q[ 5] = S32_0(q[ 5]) + (h[ 6] ^= m[ 6]);
	q[ 6] = S32_1(q[ 6]) + (h[ 7] ^= m[ 7]);
	q[ 7] = S32_2(q[ 7]) + (h[ 8] ^= m[ 8]);
	q[ 8] = S32_3(q[ 8]) + (h[ 9] ^= m[ 9]);
	q[ 9] = S32_4(q[ 9]) + (h[10] ^= m[10]);
	q[10] = S32_0(q[10]) + (h[11] ^= m[11]);
	q[11] = S32_1(q[11]) + (h[12] ^= m[12]);
	q[12] = S32_2(q[12]) + (h[13] ^= m[13]);
	q[13] = S32_3(q[13]) + (h[14] ^= m[14]);
	q[14] = S32_4(q[14]) + (h[15] ^= m[15]);
	q[15] = S32_0(q[15]) + (h[ 0] ^= m[ 0]);
}

static inline
void bmw_small_f2(uint32_t* h, uint32_t* q, const uint32_t* m){
	uint32_t xl, xh;
	xl =      q[16] ^ q[17] ^ q[18] ^ q[19] ^ q[20] ^ q[21] ^ q[22] ^ q[23];
	xh = xl ^ q[24] ^ q[25] ^ q[26] ^ q[27] ^ q[28] ^ q[29] ^ q[30] ^ q[31];
#if DEBUG
	cli_putstr("\r\n XL = ");
	cli_hexdump_rev(&xl, 4);
	cli_putstr("\r\n XH = ");
	cli_hexdump_rev(&xh, 4);
#endif

	h[0] = (SHL32(xh, 5) ^ SHR32(q[16], 5) ^ m[ 0]) + (xl ^ q[24] ^ q[ 0]);
	h[1] = (SHR32(xh, 7) ^ SHL32(q[17], 8) ^ m[ 1]) + (xl ^ q[25] ^ q[ 1]);
	h[2] = (SHR32(xh, 5) ^ SHL32(q[18], 5) ^ m[ 2]) + (xl ^ q[26] ^ q[ 2]);
	h[3] = (SHR32(xh, 1) ^ SHL32(q[19], 5) ^ m[ 3]) + (xl ^ q[27] ^ q[ 3]);
	h[4] = (SHR32(xh, 3) ^ q[20]           ^ m[ 4]) + (xl ^ q[28] ^ q[ 4]);
	h[5] = (SHL32(xh, 6) ^ SHR32(q[21], 6) ^ m[ 5]) + (xl ^ q[29] ^ q[ 5]);
	h[6] = (SHR32(xh, 4) ^ SHL32(q[22], 6) ^ m[ 6]) + (xl ^ q[30] ^ q[ 6]);
	h[7] = (SHR32(xh,11) ^ SHL32(q[23], 2) ^ m[ 7]) + (xl ^ q[31] ^ q[ 7]);

	h[ 8] = ROTL32(h[4],  9) + (xh ^ q[24] ^ m[ 8]) + (SHL32(xl, 8) ^ q[23] ^ q[ 8]);
	h[ 9] = ROTL32(h[5], 10) + (xh ^ q[25] ^ m[ 9]) + (SHR32(xl, 6) ^ q[16] ^ q[ 9]);
	h[10] = ROTL32(h[6], 11) + (xh ^ q[26] ^ m[10]) + (SHL32(xl, 6) ^ q[17] ^ q[10]);
	h[11] = ROTL32(h[7], 12) + (xh ^ q[27] ^ m[11]) + (SHL32(xl, 4) ^ q[18] ^ q[11]);
	h[12] = ROTL32(h[0], 13) + (xh ^ q[28] ^ m[12]) + (SHR32(xl, 3) ^ q[19] ^ q[12]);
	h[13] = ROTL32(h[1], 14) + (xh ^ q[29] ^ m[13]) + (SHR32(xl, 4) ^ q[20] ^ q[13]);
	h[14] = ROTL32(h[2], 15) + (xh ^ q[30] ^ m[14]) + (SHR32(xl, 7) ^ q[21] ^ q[14]);
	h[15] = ROTL32(h[3], 16) + (xh ^ q[31] ^ m[15]) + (SHR32(xl, 2) ^ q[22] ^ q[15]);
}

void bmw_small_nextBlock(bmw_small_ctx_t* ctx, const void* block){
	uint32_t q[32];
	dump_x(block, 16, 'M');
	bmw_small_f0(q, ctx->h, block);
	dump_x(q, 16, 'Q');
	bmw_small_f1(q, block, ctx->h);
	dump_x(q+16, 16, 'Q');
	bmw_small_f2(ctx->h, q, block);
	ctx->counter += 1;
	ctx_dump(ctx);
}

void bmw_small_lastBlock(bmw_small_ctx_t* ctx, const void* block, uint16_t length_b){
	uint8_t buffer[64];
	while(length_b >= BMW_SMALL_BLOCKSIZE){
		bmw_small_nextBlock(ctx, block);
		length_b -= BMW_SMALL_BLOCKSIZE;
		block = (uint8_t*)block + BMW_SMALL_BLOCKSIZE_B;
	}
	memset(buffer, 0, 64);
	memcpy(buffer, block, (length_b+7)/8);
	buffer[length_b>>3] |= 0x80 >> (length_b&0x07);
	if(length_b+1>64*8-64){
		bmw_small_nextBlock(ctx, buffer);
		memset(buffer, 0, 64-8);
		ctx->counter -= 1;
	}
	*((uint64_t*)&(buffer[64-8])) = (uint64_t)(ctx->counter*512LL)+(uint64_t)length_b;
	bmw_small_nextBlock(ctx, buffer);
	uint8_t i;
	uint32_t q[32];
	memset(buffer, 0xaa, 64);
	for(i=0; i<16;++i){
		buffer[i*4] = i+0xa0;
	}
//	dump_x(buffer, 16, 'A');
	dump_x(ctx->h, 16, 'M');
	bmw_small_f0(q, (uint32_t*)buffer, ctx->h);
	dump_x(buffer, 16, 'a');
	dump_x(q, 16, 'Q');
	bmw_small_f1(q, ctx->h, (uint32_t*)buffer);
	dump_x(q, 32, 'Q');
	bmw_small_f2((uint32_t*)buffer, q, ctx->h);
	memcpy(ctx->h, buffer, 64);
}

void bmw224_init(bmw224_ctx_t* ctx){
	uint8_t i;
	ctx->h[0] = 0x00010203;
	for(i=1; i<16; ++i){
		ctx->h[i] = ctx->h[i-1]+ 0x04040404;
	}
	ctx->counter=0;
	ctx_dump(ctx);
}

void bmw256_init(bmw256_ctx_t* ctx){
	uint8_t i;
	ctx->h[0] = 0x40414243;
	for(i=1; i<16; ++i){
		ctx->h[i] = ctx->h[i-1]+ 0x04040404;
	}
	ctx->counter=0;
	ctx_dump(ctx);
}

void bmw224_nextBlock(bmw224_ctx_t* ctx, const void* block){
	bmw_small_nextBlock(ctx, block);
}

void bmw256_nextBlock(bmw256_ctx_t* ctx, const void* block){
	bmw_small_nextBlock(ctx, block);
}

void bmw224_lastBlock(bmw224_ctx_t* ctx, const void* block, uint16_t length_b){
	bmw_small_lastBlock(ctx, block, length_b);
}

void bmw256_lastBlock(bmw256_ctx_t* ctx, const void* block, uint16_t length_b){
	bmw_small_lastBlock(ctx, block, length_b);
}

void bmw224_ctx2hash(void* dest, const bmw224_ctx_t* ctx){
	memcpy(dest, &(ctx->h[9]), 224/8);
}

void bmw256_ctx2hash(void* dest, const bmw256_ctx_t* ctx){
	memcpy(dest, &(ctx->h[8]), 256/8);
}

void bmw224(void* dest, const void* msg, uint32_t length_b){
	bmw_small_ctx_t ctx;
	bmw224_init(&ctx);
	while(length_b>=BMW_SMALL_BLOCKSIZE){
		bmw_small_nextBlock(&ctx, msg);
		length_b -= BMW_SMALL_BLOCKSIZE;
		msg = (uint8_t*)msg + BMW_SMALL_BLOCKSIZE_B;
	}
	bmw_small_lastBlock(&ctx, msg, length_b);
	bmw224_ctx2hash(dest, &ctx);
}

void bmw256(void* dest, const void* msg, uint32_t length_b){
	bmw_small_ctx_t ctx;
	bmw256_init(&ctx);
	while(length_b>=BMW_SMALL_BLOCKSIZE){
		bmw_small_nextBlock(&ctx, msg);
		length_b -= BMW_SMALL_BLOCKSIZE;
		msg = (uint8_t*)msg + BMW_SMALL_BLOCKSIZE_B;
	}
	bmw_small_lastBlock(&ctx, msg, length_b);
	bmw256_ctx2hash(dest, &ctx);
}

#include "f1_autogen.c"

