dnl  SPARC v9 64-bit mpn_sqr_diagonal.

dnl  Copyright 2001, 2002 Free Software Foundation, Inc.

dnl  This file is part of the GNU MP Library.

dnl  The GNU MP Library is free software; you can redistribute it and/or modify
dnl  it under the terms of the GNU Lesser General Public License as published
dnl  by the Free Software Foundation; either version 3 of the License, or (at
dnl  your option) any later version.

dnl  The GNU MP Library is distributed in the hope that it will be useful, but
dnl  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
dnl  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
dnl  License for more details.

dnl  You should have received a copy of the GNU Lesser General Public License
dnl  along with the GNU MP Library.  If not, see http://www.gnu.org/licenses/.

include(`../config.m4')

C		   cycles/limb
C UltraSPARC 1&2:     22
C UltraSPARC 3:	      36

C This was generated by the Sun C compiler.  It runs at 22 cycles/limb on the
C UltraSPARC-1/2, three cycles slower than theoretically possible for optimal
C code using the same algorithm.  For 1-3 limbs, a special loop was generated,
C which causes performance problems in particular for 2 and 3 limbs.
C Ultimately, this should be replaced by hand-written code in the same software
C pipeline style as e.g., addmul_1.asm.

ASM_START()
	REGISTER(%g2,#scratch)
	REGISTER(%g3,#scratch)
PROLOGUE(mpn_sqr_diagonal)
	save	%sp, -240, %sp

	sethi	%hi(0x1ffc00), %o0
	sethi	%hi(0x3ffc00), %o1
	add	%o0, 1023, %o7
	cmp	%i2, 4
	add	%o1, 1023, %o4
	or	%g0, %i1, %g1
	or	%g0, %i0, %o0
	bl,pn	%xcc, .Lsmall
	or	%g0, 0, %g2

	ldx	[%i1], %o1
	add	%i1, 24, %g1
	or	%g0, 3, %g2
	srlx	%o1, 42, %g3
	stx	%g3, [%sp+2279]
	and	%o1, %o7, %o2
	stx	%o2, [%sp+2263]
	srlx	%o1, 21, %o1
	ldd	[%sp+2279], %f0
	and	%o1, %o7, %o1
	stx	%o1, [%sp+2271]
	ldx	[%i1+8], %o2
	fxtod	%f0, %f12
	srlx	%o2, 21, %o1
	and	%o2, %o7, %g3
	ldd	[%sp+2263], %f2
	fmuld	%f12, %f12, %f10
	srlx	%o2, 42, %o2
	ldd	[%sp+2271], %f0
	and	%o1, %o7, %o1
	fxtod	%f2, %f8
	stx	%o2, [%sp+2279]
	stx	%o1, [%sp+2271]
	fxtod	%f0, %f0
	stx	%g3, [%sp+2263]
	fdtox	%f10, %f14
	fmuld	%f12, %f8, %f6
	ldx	[%i1+16], %o2
	std	%f14, [%sp+2255]
	fmuld	%f0, %f0, %f2
	fmuld	%f8, %f8, %f10
	srlx	%o2, 42, %o1
	faddd	%f6, %f6, %f6
	fmuld	%f12, %f0, %f12
	fmuld	%f0, %f8, %f8
	ldd	[%sp+2279], %f0
	ldd	[%sp+2263], %f4
	fdtox	%f10, %f10
	std	%f10, [%sp+2239]
	faddd	%f2, %f6, %f6
	ldd	[%sp+2271], %f2
	fdtox	%f12, %f12
	std	%f12, [%sp+2247]
	fdtox	%f8, %f8
	std	%f8, [%sp+2231]
	fdtox	%f6, %f6
	std	%f6, [%sp+2223]

.Loop:	srlx	%o2, 21, %g3
	stx	%o1, [%sp+2279]
	add	%g2, 1, %g2
	and	%g3, %o7, %o1
	ldx	[%sp+2255], %g4
	cmp	%g2, %i2
	stx	%o1, [%sp+2271]
	add	%g1, 8, %g1
	add	%o0, 16, %o0
	ldx	[%sp+2239], %o1
	fxtod	%f0, %f10
	fxtod	%f4, %f14
	ldx	[%sp+2231], %i0
	ldx	[%sp+2223], %g5
	ldx	[%sp+2247], %g3
	and	%o2, %o7, %o2
	fxtod	%f2, %f8
	fmuld	%f10, %f10, %f0
	stx	%o2, [%sp+2263]
	fmuld	%f10, %f14, %f6
	ldx	[%g1-8], %o2
	fmuld	%f10, %f8, %f12
	fdtox	%f0, %f2
	ldd	[%sp+2279], %f0
	fmuld	%f8, %f8, %f4
	faddd	%f6, %f6, %f6
	fmuld	%f14, %f14, %f10
	std	%f2, [%sp+2255]
	sllx	%g4, 20, %g4
	ldd	[%sp+2271], %f2
	fmuld	%f8, %f14, %f8
	sllx	%i0, 22, %i1
	fdtox	%f12, %f12
	std	%f12, [%sp+2247]
	sllx	%g5, 42, %i0
	add	%o1, %i1, %o1
	faddd	%f4, %f6, %f6
	ldd	[%sp+2263], %f4
	add	%o1, %i0, %o1
	add	%g3, %g4, %g3
	fdtox	%f10, %f10
	std	%f10, [%sp+2239]
	srlx	%o1, 42, %g4
	and	%g5, %o4, %i0
	fdtox	%f8, %f8
	std	%f8, [%sp+2231]
	srlx	%g5, 22, %g5
	sub	%g4, %i0, %g4
	fdtox	%f6, %f6
	std	%f6, [%sp+2223]
	srlx	%g4, 63, %g4
	add	%g3, %g5, %g3
	add	%g3, %g4, %g3
	stx	%o1, [%o0-16]
	srlx	%o2, 42, %o1
	bl,pt	%xcc, .Loop
	stx	%g3, [%o0-8]

	stx	%o1, [%sp+2279]
	srlx	%o2, 21, %o1
	fxtod	%f0, %f16
	ldx	[%sp+2223], %g3
	fxtod	%f4, %f6
	and	%o2, %o7, %o3
	stx	%o3, [%sp+2263]
	fxtod	%f2, %f4
	and	%o1, %o7, %o1
	ldx	[%sp+2231], %o2
	sllx	%g3, 42, %g4
	fmuld	%f16, %f16, %f14
	stx	%o1, [%sp+2271]
	fmuld	%f16, %f6, %f8
	add	%o0, 48, %o0
	ldx	[%sp+2239], %o1
	sllx	%o2, 22, %o2
	fmuld	%f4, %f4, %f10
	ldx	[%sp+2255], %o3
	fdtox	%f14, %f14
	fmuld	%f4, %f6, %f2
	std	%f14, [%sp+2255]
	faddd	%f8, %f8, %f12
	add	%o1, %o2, %o2
	fmuld	%f16, %f4, %f4
	ldd	[%sp+2279], %f0
	sllx	%o3, 20, %g5
	add	%o2, %g4, %o2
	fmuld	%f6, %f6, %f6
	srlx	%o2, 42, %o3
	and	%g3, %o4, %g4
	srlx	%g3, 22, %g3
	faddd	%f10, %f12, %f16
	ldd	[%sp+2271], %f12
	ldd	[%sp+2263], %f8
	fxtod	%f0, %f0
	sub	%o3, %g4, %o3
	ldx	[%sp+2247], %o1
	srlx	%o3, 63, %o3
	fdtox	%f2, %f10
	fxtod	%f8, %f8
	std	%f10, [%sp+2231]
	fdtox	%f6, %f6
	std	%f6, [%sp+2239]
	add	%o1, %g5, %o1
	fmuld	%f0, %f0, %f2
	fdtox	%f16, %f16
	std	%f16, [%sp+2223]
	add	%o1, %g3, %o1
	fdtox	%f4, %f4
	std	%f4, [%sp+2247]
	fmuld	%f0, %f8, %f10
	fxtod	%f12, %f12
	add	%o1, %o3, %o1
	stx	%o2, [%o0-48]
	fmuld	%f8, %f8, %f6
	stx	%o1, [%o0-40]
	fdtox	%f2, %f2
	ldx	[%sp+2231], %o2
	faddd	%f10, %f10, %f10
	ldx	[%sp+2223], %g3
	fmuld	%f12, %f12, %f4
	fdtox	%f6, %f6
	ldx	[%sp+2239], %o1
	sllx	%o2, 22, %o2
	fmuld	%f12, %f8, %f8
	sllx	%g3, 42, %g5
	ldx	[%sp+2255], %o3
	fmuld	%f0, %f12, %f0
	add	%o1, %o2, %o2
	faddd	%f4, %f10, %f4
	ldx	[%sp+2247], %o1
	add	%o2, %g5, %o2
	and	%g3, %o4, %g4
	fdtox	%f8, %f8
	sllx	%o3, 20, %g5
	std	%f8, [%sp+2231]
	fdtox	%f0, %f0
	srlx	%o2, 42, %o3
	add	%o1, %g5, %o1
	fdtox	%f4, %f4
	srlx	%g3, 22, %g3
	sub	%o3, %g4, %o3
	std	%f6, [%sp+2239]
	std	%f4, [%sp+2223]
	srlx	%o3, 63, %o3
	add	%o1, %g3, %o1
	std	%f2, [%sp+2255]
	add	%o1, %o3, %o1
	std	%f0, [%sp+2247]
	stx	%o2, [%o0-32]
	stx	%o1, [%o0-24]
	ldx	[%sp+2231], %o2
	ldx	[%sp+2223], %o3
	ldx	[%sp+2239], %o1
	sllx	%o2, 22, %o2
	sllx	%o3, 42, %g5
	ldx	[%sp+2255], %g4
	and	%o3, %o4, %g3
	add	%o1, %o2, %o2
	ldx	[%sp+2247], %o1
	add	%o2, %g5, %o2
	stx	%o2, [%o0-16]
	sllx	%g4, 20, %g4
	srlx	%o2, 42, %o2
	add	%o1, %g4, %o1
	srlx	%o3, 22, %o3
	sub	%o2, %g3, %o2
	srlx	%o2, 63, %o2
	add	%o1, %o3, %o1
	add	%o1, %o2, %o1
	stx	%o1, [%o0-8]
	ret
	restore	%g0, %g0, %g0
.Lsmall:
	ldx	[%g1], %o2
.Loop0:
	and	%o2, %o7, %o1
	stx	%o1, [%sp+2263]
	add	%g2, 1, %g2
	srlx	%o2, 21, %o1
	add	%g1, 8, %g1
	srlx	%o2, 42, %o2
	stx	%o2, [%sp+2279]
	and	%o1, %o7, %o1
	ldd	[%sp+2263], %f0
	cmp	%g2, %i2
	stx	%o1, [%sp+2271]
	fxtod	%f0, %f6
	ldd	[%sp+2279], %f0
	ldd	[%sp+2271], %f4
	fxtod	%f0, %f2
	fmuld	%f6, %f6, %f0
	fxtod	%f4, %f10
	fmuld	%f2, %f6, %f4
	fdtox	%f0, %f0
	std	%f0, [%sp+2239]
	fmuld	%f10, %f6, %f8
	fmuld	%f10, %f10, %f0
	faddd	%f4, %f4, %f6
	fmuld	%f2, %f2, %f4
	fdtox	%f8, %f8
	std	%f8, [%sp+2231]
	fmuld	%f2, %f10, %f2
	faddd	%f0, %f6, %f0
	fdtox	%f4, %f4
	std	%f4, [%sp+2255]
	fdtox	%f2, %f2
	std	%f2, [%sp+2247]
	fdtox	%f0, %f0
	std	%f0, [%sp+2223]
	ldx	[%sp+2239], %o1
	ldx	[%sp+2255], %g4
	ldx	[%sp+2231], %o2
	sllx	%g4, 20, %g4
	ldx	[%sp+2223], %o3
	sllx	%o2, 22, %o2
	sllx	%o3, 42, %g5
	add	%o1, %o2, %o2
	ldx	[%sp+2247], %o1
	add	%o2, %g5, %o2
	stx	%o2, [%o0]
	and	%o3, %o4, %g3
	srlx	%o2, 42, %o2
	add	%o1, %g4, %o1
	srlx	%o3, 22, %o3
	sub	%o2, %g3, %o2
	srlx	%o2, 63, %o2
	add	%o1, %o3, %o1
	add	%o1, %o2, %o1
	stx	%o1, [%o0+8]
	add	%o0, 16, %o0
	bl,a,pt	%xcc, .Loop0
	ldx	[%g1], %o2
	ret
	restore	%g0, %g0, %g0
EPILOGUE(mpn_sqr_diagonal)
