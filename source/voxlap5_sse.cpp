// SSE Specific Code
namespace cpu_sse_specific_code
{
	#ifndef _DOS
	void hrendz (long sx, long sy, long p1, long plc, long incr, long j)
	{
		#if defined(__GNUC__) && !defined(__NOASM__) //AT&T SYNTAX ASSEMBLY
		__asm__ __volatile__
		(
			"push	%esi\n\t"
			"push	%edi\n"
	"beghasm_p3:\n\t"
			"mov	sx,%eax\n\t"
			"mov	sy,%ecx\n\t"
			"mov	p1,%esi\n\t"
			"mov	ylookup(,%ecx,4),%edx\n\t"
			"add	frameplace,%edx\n\t"
			"lea	(%edx,%eax,4),%edi\n\t"
			"lea	(%edx,%esi,4),%esi\n\t"
	
			"and	$0xfffffffc,%eax\n\t"
			"cvtsi2ss	%eax,%xmm0\n\t"
			"cvtsi2ss	%ecx,%xmm4\n\t"
			"movss	%xmm0,%xmm1\n\t"
			"movss	%xmm4,%xmm5\n\t"
			"mulss	optistrx,%xmm0\n\t"
			"mulss	optistry,%xmm1\n\t"
			"mulss	optiheix,%xmm4\n\t"
			"mulss	optiheiy,%xmm5\n\t"
			"addss	optiaddx,%xmm0\n\t"
			"addss	optiaddy,%xmm1\n\t"
			"addss	%xmm4,%xmm0\n\t"
			"addss	%xmm5,%xmm1\n\t"
	
			"mov	zbufoff,%ecx\n\t"
			"mov	j,%edx\n\t"
			"movl	plc,%mm6\n\t"
			"movl	incr,%mm7\n\t"
	
			"shufps	%xmm0,0,%xmm0\n\t"
			"shufps	%xmm1,0,%xmm1\n\t"
			"movaps	opti4asm+2*16,%xmm2\n\t"
			"movaps	opti4asm+3*16,%xmm3\n\t"
			"addps	opti4asm+0*16,%xmm0\n\t"
			"addps	opti4asm+1*16,%xmm1\n\t"
					//xmm0 =  xmm0      ^2 +  xmm1      ^2        (p)\n\t"
					//xmm2 = (xmm0+xmm2)^2 + (xmm1+xmm3)^2 - xmm0 (v)\n\t"
					//xmm1 = ...                                  (a)\n\t"
			"addps	%xmm0,%xmm2 \n\t" //This block converts inner loop...
			"addps	%xmm1,%xmm3 \n\t" //from: 1 / sqrt(x*x + y*y), x += xi, y += yi;
			"mulps	%xmm0,%xmm0 \n\t" //  to: 1 / sqrt(p), p += v, v += a;
			"mulps	%xmm1,%xmm1\n\t"
			"mulps	%xmm2,%xmm2\n\t"
			"mulps	%xmm3,%xmm3\n\t"
			"addps	%xmm1,%xmm0\n\t"
			"movaps	opti4asm+4*16,%xmm1\n\t"
			"addps	%xmm3,%xmm2\n\t"
			"subps	%xmm0,%xmm2\n\t"
	
					//Do first 0-3 pixels to align unrolled loop of 4\n\t"
			"test	$15,%edi\n\t"
			"jz	skip1ha\n\t"
	
			"test	$8,%edi\n\t"
			"jz	skipshufa\n\t"
			"shufps	%xmm0,0x4e,%xmm0\n" //rotate right by 2
	"skipshufa:\n\t"
			"test	$4,%edi\n\t"
			"jz	skipshufb\n\t"
			"shufps	%xmm0,0x39,%xmm0\n" //rotate right by 1
	"skipshufb:\n"
	
	"beg1ha:\n\t"
			"pextrw	%mm6,1,%eax\n\t"
			"paddd	%mm7,%mm6\n\t"
			"mov	angstart(,%eax,4),%eax\n\t"
			"movl	(%eax,%edx,8),%mm0\n\t"
			"movl	%mm0,(%edi)\n\t"
			"cvtsi2ss	4(%eax,%edx,8),%xmm7\n\t"
			"rsqrtss	%xmm0,%xmm3\n\t"
			"mulss	%xmm3,%xmm7\n\t"
			"shufps	%xmm0,0x39,%xmm0\n\t" //rotate right by 1
			"movss	%xmm7,(%edi,%ecx)\n\t"
			"add	$4,%edi\n\t"
			"cmp	%esi,%edi\n\t"
			"jz	endh\n\t"
			"test	$15,%edi\n\t"
			"jnz	beg1ha\n\t"
	
			"addps	%xmm2,%xmm0\n\t"
			"addps	%xmm1,%xmm2\n"
	"skip1ha:\n\t"
			"lea	16(%edi),%eax     \n\t" //these 3 lines re-ordered
			"cmp	%esi,%eax\n\t"
			"ja	skip4h\n\t"
	
			"movq	%mm6,%mm0         \n\t" //mm0: 0,plc
			"paddd	%mm7,%mm0        \n\t" //mm0: 0,plc+inc
			"punpckldq	%mm7,%mm7    \n\t" //mm7: inc,inc
			"punpckldq	%mm0,%mm6    \n\t" //mm6: plc+inc,plc
			"paddd	%mm7,%mm7        \n\t" //mm7: inc+inc,inc+inc
	
			"sub	$16,%esi\n"
	
				//eax: temp   ³ mm0:  z0 argb0   argb1 argb0 ³ xmm0: plc3 plc2 plc1 plc0
				//ebx:  -     ³ mm1:  z1 argb1               ³ xmm1: acc3 acc2 acc1 acc0
				//ecx:zbufoff ³ mm2:  z2 argb2   argb3 argb2 ³ xmm2: inc3 inc2 inc1 inc0
				//edx:  j     ³ mm3:  z3 argb3               ³ xmm3:  r3   r2   r1   r0
				//esi:  -     ³ mm4:              z1    z0   ³ xmm4:            z3   z2
				//edi:scroff  ³ mm5:              z3    z2   ³ xmm5:
				//ebp:  -     ³ mm6: plc1 plc0               ³ xmm6:
	"beg4h:\n\t"
				//esp:  -     ³ mm7: inc1 inc0               ³ xmm7:  z3   z2   z1   z0\n\t"
			"pextrw	%mm6,1,%eax\n\t"
			"mov	angstart(,%eax,4),%eax\n\t"
			"movq	(%eax,%edx,8),%mm0\n\t"
			"pextrw	%mm6,3,%eax\n\t"
			"mov	angstart(,%eax,4),%eax\n\t"
			"movq	(%eax,%edx,8),%mm1\n\t"
			"paddd	%mm7,%mm6\n\t"
			"pextrw	%mm6,1,%eax\n\t"
			"mov	angstart(,%eax,4),%eax\n\t"
			"movq	(%eax,%edx,8),%mm2\n\t"
			"pextrw	%mm6,3,%eax\n\t"
			"mov	angstart(,%eax,4),%eax\n\t"
			"movq	(%eax,%edx,8),%mm3\n\t"
			"paddd	%mm7,%mm6\n\t"
	
			"movq	%mm0,%mm4\n\t"
			"movq	%mm2,%mm5\n\t"
			"punpckldq	%mm1,%mm0\n\t"
			"punpckldq	%mm3,%mm2\n\t"
			"movntq	%mm0,(%edi)\n\t"
			"movntq	%mm2,8(%edi)\n\t"
	
			"punpckhdq	%mm1,%mm4\n\t"
			"punpckhdq	%mm3,%mm5\n\t"
			"cvtpi2ps	%mm4,%xmm7\n\t"
			"cvtpi2ps	%mm5,%xmm4\n\t"
			"rsqrtps	%xmm0,%xmm3\n\t"
			"movlhps	%xmm4,%xmm7\n\t"
			"mulps	%xmm3,%xmm7\n\t"
			"movntps	%xmm7,(%edi,%ecx)\n\t"
			"addps	%xmm2,%xmm0\n\t"
			"addps	%xmm1,%xmm2\n\t"
	
			"add	$16,%edi\n\t"
			"cmp	%esi,%edi\n\t"
			"jbe	beg4h\n\t"
			"add	$16,%esi\n\t"
			"cmp	%esi,%edi\n\t"
			"jae	endh\n\t"
	
			"psrad	$1,%mm7\n"    //Restore mm7 from incr*2 to just incr for single loop
	"skip4h:\n"
	"beg1h:\n\t"
			"pextrw	%mm6,1,%eax\n\t"
			"paddd	%mm7,%mm6\n\t"
			"mov	angstart(,%eax,4),%eax\n\t"
			"movl	(%eax,%edx,8),%mm0\n\t"
			"movl	%mm0,(%edi)\n\t"
			"cvtsi2ss	4(%eax,%edx,8),%xmm7\n\t"
			"rsqrtss	%xmm0,%xmm3\n\t"
			"mulss	%xmm3,%xmm7\n\t"
			"shufps	%xmm0,0x39,%xmm0\n\t" //rotate right by 1
			"movss	%xmm7,(%edi,%ecx)\n\t"
			"add	$4,%edi\n\t"
			"cmp	%esi,%edi\n\t"
			"jb	beg1h\n"
	"endh:\n\t"
			"pop	%edi\n\t"
			"pop	%esi\n\t"
		);
		#endif
		#if defined(_MSC_VER) && !defined(__NOASM__) //MASM SYNTAX ASSEMBLY
		_asm
		{
			push	esi
			push	edi
		beghasm_p3:
			mov	eax, sx
			mov	ecx, sy
			mov	esi, p1
			mov	edx, ylookup[ecx*4]
			add	edx, frameplace
			lea	edi, [edx+eax*4]
			lea	esi, [edx+esi*4]
	
			and	eax, 0xfffffffc
			cvtsi2ss	xmm0, eax
			cvtsi2ss	xmm4, ecx
			movss	xmm1, xmm0
			movss	xmm5, xmm4
			mulss	xmm0, optistrx
			mulss	xmm1, optistry
			mulss	xmm4, optiheix
			mulss	xmm5, optiheiy
			addss	xmm0, optiaddx
			addss	xmm1, optiaddy
			addss	xmm0, xmm4
			addss	xmm1, xmm5
	
			mov	ecx, zbufoff
			mov	edx, j
			movd	mm6, plc
			movd	mm7, incr
	
			shufps	xmm0, xmm0, 0
			shufps	xmm1, xmm1, 0
			movaps	xmm2, opti4asm[2*16]
			movaps	xmm3, opti4asm[3*16]
			addps	xmm0, opti4asm[0*16]
			addps	xmm1, opti4asm[1*16]
				//xmm0 =  xmm0      ^2 +  xmm1      ^2        (p)
				//xmm2 = (xmm0+xmm2)^2 + (xmm1+xmm3)^2 - xmm0 (v)
				//xmm1 = ...                                  (a)
			addps	xmm2, xmm0  //This block converts inner loop...
			addps	xmm3, xmm1  //from: 1 / sqrt(x*x + y*y), x += xi, y += yi;
			mulps	xmm0, xmm0  //  to: 1 / sqrt(p), p += v, v += a;
			mulps	xmm1, xmm1
			mulps	xmm2, xmm2
			mulps	xmm3, xmm3
			addps	xmm0, xmm1
			movaps	xmm1, opti4asm[4*16]
			addps	xmm2, xmm3
			subps	xmm2, xmm0
	
				//Do first 0-3 pixels to align unrolled loop of 4
			test	edi, 15
			jz	short skip1ha
	
			test	edi, 8
			jz	short skipshufa
			shufps	xmm0, xmm0, 0x4e //rotate right by 2
		skipshufa:
			test	edi, 4
			jz	short skipshufb
			shufps	xmm0, xmm0, 0x39 //rotate right by 1
		skipshufb:
	
		beg1ha:
			pextrw	eax, mm6, 1
			paddd	mm6, mm7
			mov	eax, angstart[eax*4]
			movd	mm0, [eax+edx*8]
			movd	[edi], mm0
			cvtsi2ss	xmm7, [eax+edx*8+4]
			rsqrtss	xmm3, xmm0
			mulss	xmm7, xmm3
			shufps	xmm0, xmm0, 0x39 //rotate right by 1
			movss	[edi+ecx], xmm7
			add	edi, 4
			cmp	edi, esi
			jz	short endh
			test	edi, 15
			jnz	short beg1ha
	
			addps	xmm0, xmm2
			addps	xmm2, xmm1
		skip1ha:
			lea	eax, [edi+16]    //these 3 lines re-ordered
			cmp	eax, esi
			ja	short skip4h
	
			movq	mm0, mm6     //mm0: 0,plc
			paddd	mm0, mm7     //mm0: 0,plc+inc
			punpckldq	mm7, mm7 //mm7: inc,inc
			punpckldq	mm6, mm0 //mm6: plc+inc,plc
			paddd	mm7, mm7     //mm7: inc+inc,inc+inc
	
			sub	esi, 16
	
				//eax: temp   ³ mm0:  z0 argb0   argb1 argb0 ³ xmm0: plc3 plc2 plc1 plc0
				//ebx:  -     ³ mm1:  z1 argb1               ³ xmm1: acc3 acc2 acc1 acc0
				//ecx:zbufoff ³ mm2:  z2 argb2   argb3 argb2 ³ xmm2: inc3 inc2 inc1 inc0
				//edx:  j     ³ mm3:  z3 argb3               ³ xmm3:  r3   r2   r1   r0
				//esi:  -     ³ mm4:              z1    z0   ³ xmm4:            z3   z2
				//edi:scroff  ³ mm5:              z3    z2   ³ xmm5:
				//ebp:  -     ³ mm6: plc1 plc0               ³ xmm6:
		beg4h:
				//esp:  -     ³ mm7: inc1 inc0               ³ xmm7:  z3   z2   z1   z0
			pextrw	eax, mm6, 1
			mov	eax, angstart[eax*4]
			movq	mm0, [eax+edx*8]
			pextrw	eax, mm6, 3
			mov	eax, angstart[eax*4]
			movq	mm1, [eax+edx*8]
			paddd	mm6, mm7
			pextrw	eax, mm6, 1
			mov	eax, angstart[eax*4]
			movq	mm2, [eax+edx*8]
			pextrw	eax, mm6, 3
			mov	eax, angstart[eax*4]
			movq	mm3, [eax+edx*8]
			paddd	mm6, mm7
	
			movq	mm4, mm0
			movq	mm5, mm2
			punpckldq	mm0, mm1
			punpckldq	mm2, mm3
			movntq	[edi], mm0
			movntq	[edi+8], mm2
	
			punpckhdq	mm4, mm1
			punpckhdq	mm5, mm3
			cvtpi2ps	xmm7, mm4
			cvtpi2ps	xmm4, mm5
			rsqrtps	xmm3, xmm0
			movlhps	xmm7, xmm4
			mulps	xmm7, xmm3
			movntps	[edi+ecx], xmm7
			addps	xmm0, xmm2
			addps	xmm2, xmm1
	
			add	edi, 16
			cmp	edi, esi
			jbe	short beg4h
			add	esi, 16
			cmp	edi, esi
			jae	endh
	
			psrad	mm7, 1    //Restore mm7 from incr*2 to just incr for single loop
		skip4h:
		beg1h:
			pextrw	eax, mm6, 1
			paddd	mm6, mm7
			mov	eax, angstart[eax*4]
			movd	mm0, [eax+edx*8]
			movd	[edi], mm0
			cvtsi2ss	xmm7, [eax+edx*8+4]
			rsqrtss	xmm3, xmm0
			mulss	xmm7, xmm3
			shufps	xmm0, xmm0, 0x39 //rotate right by 1
			movss	[edi+ecx], xmm7
			add	edi, 4
			cmp	edi, esi
			jb	short beg1h
		endh:
			pop	edi
			pop	esi
		}
		#endif
	}
	
	void hrendzfog (long sx, long sy, long p1, long plc, long incr, long j)
	{
		static int64_t mm7bak;
		#if defined(__GNUC__) && !defined(__NOASM__) //AT&T SYNTAX ASSEMBLY
		__asm__ __volatile__
		(
			"push	%esi\n\t"
			"push	%edi\n"
	"beghasm_p3:\n\t"
			"mov	sx,%eax\n\t"
			"mov	sy,%ecx\n\t"
			"mov	p1,%esi\n\t"
			"mov	ylookup(,%ecx,4),%edx\n\t"
			"add	frameplace,%edx\n\t"
			"lea	(%edx,%eax,4),%edi\n\t"
			"lea	(%edx,%esi,4),%esi\n\t"
	
			"and	$0xfffffffc,%eax\n\t"
			"cvtsi2ss	%eax,%xmm0\n\t"
			"cvtsi2ss	%ecx,%xmm4\n\t"
			"movss	%xmm0,%xmm1\n\t"
			"movss	%xmm4,%xmm5\n\t"
			"mulss	optistrx,%xmm0\n\t"
			"mulss	optistry,%xmm1\n\t"
			"mulss	optiheix,%xmm4\n\t"
			"mulss	optiheiy,%xmm5\n\t"
			"addss	optiaddx,%xmm0\n\t"
			"addss	optiaddy,%xmm1\n\t"
			"addss	%xmm4,%xmm0\n\t"
			"addss	%xmm5,%xmm1\n\t"
	
			"mov	zbufoff,%ecx\n\t"
			"mov	j,%edx\n\t"
			"movl	plc,%mm6\n\t"
			"movl	incr,%mm7\n\t"
	
			"shufps	%xmm0,0,%xmm0\n\t"
			"shufps	%xmm1,0,%xmm1\n\t"
			"movaps	opti4asm+2*16,%xmm2\n\t"
			"movaps	opti4asm+3*16,%xmm3\n\t"
			"addps	opti4asm+0*16,%xmm0\n\t"
			"addps	opti4asm+1*16,%xmm1\n\t"
			//xmm0 =  xmm0      ^2 +  xmm1      ^2        (p)
			//xmm2 = (xmm0+xmm2)^2 + (xmm1+xmm3)^2 - xmm0 (v)
			//xmm1 = ...                                  (a)
			"addps	%xmm0,%xmm2 \n\t" //This block converts inner loop...
			"addps	%xmm1,%xmm3 \n\t" //from: 1 / sqrt(x*x + y*y), x += xi, y += yi;
			"mulps	%xmm0,%xmm0 \n\t" //  to: 1 / sqrt(p), p += v, v += a;
			"mulps	%xmm1,%xmm1\n\t"
			"mulps	%xmm2,%xmm2\n\t"
			"mulps	%xmm3,%xmm3\n\t"
			"addps	%xmm1,%xmm0\n\t"
			"movaps	opti4asm+4*16,%xmm1\n\t"
			"addps	%xmm3,%xmm2\n\t"
			"subps	%xmm0,%xmm2\n\t"
	
			//Do first 0-3 pixels to align unrolled loop of 4\n\t"
			"test	$15,%edi\n\t"
			"jz	skip1ha\n\t"
	
			"test	$8,%edi\n\t"
			"jz	skipshufa\n\t"
			"shufps	%xmm0,0x4e,%xmm0\n" //rotate right by 2
	"skipshufa:\n\t"
			"test	$4,%edi\n\t"
			"jz	skipshufb\n\t"
			"shufps	%xmm0,0x39,%xmm0\n" //rotate right by 1
	"skipshufb:\n"
	
	"beg1ha:\n\t"
			"pextrw	%mm6,1,%eax\n\t"
			"paddd	%mm7,%mm6\n\t"
			"mov	angstart(,%eax,4),%eax\n\t"
	
			//Z
			"cvtsi2ss	4(%eax,%edx,8),%xmm7\n\t"
			"rsqrtss	%xmm0,%xmm3\n\t"
			"mulss	%xmm3,%xmm7\n\t"
			"shufps	%xmm0,0x39,%xmm0\n\t" //rotate right by 1
			"movss	%xmm7,(%edi,%ecx)\n\t"
	
			//Col
			"punpcklbw	(%eax,%edx,8),%mm0\n\t"
			"psrlw	$8,%mm0\n\t"
			"movq	fogcol,%mm1\n\t"
			"psubw	%mm0,%mm1\n\t"
			"paddw	%mm1,%mm1\n\t"
			"mov	4(%eax,%edx,8),%eax\n\t"
			"shr	16+4,%eax\n\t"
			"pmulhw	foglut(,%eax,8),%mm1\n\t"
			"paddw	%mm1,%mm0\n\t"
			"packuswb	%mm1,%mm0\n\t"
			"movl	%mm0,(%edi)\n\t"
	
			"add	$4,%edi\n\t"
			"cmp	%esi,%edi\n\t"
			"jz	endh\n\t"
			"test	$15,%edi\n\t"
			"jnz	beg1ha\n\t"
	
			"addps	%xmm2,%xmm0\n\t"
			"addps	%xmm1,%xmm2\n\t"
	"skip1ha:\n\t"
			"lea	16(%edi),%eax\n\t" //these 3 lines re-ordered
			"cmp	%esi,%eax\n\t"
			"ja	skip4h\n\t"
	
			"movq	%mm6,%mm0\n\t"     //mm0: 0,plc
			"paddd	%mm7,%mm0\n\t"     //mm0: 0,plc+inc
			"punpckldq	%mm7,%mm7\n\t" //mm7: inc,inc
			"punpckldq	%mm0,%mm6\n\t" //mm6: plc+inc,plc
			"paddd	%mm7,%mm7\n\t"     //mm7: inc+inc,inc+inc
	
			"sub	$16,%esi\n\t"
	
				//eax: temp   ³ mm0:  z0 argb0   argb1 argb0 ³ xmm0: plc3 plc2 plc1 plc0
				//ebx:  -     ³ mm1:  z1 argb1               ³ xmm1: acc3 acc2 acc1 acc0
				//ecx:zbufoff ³ mm2:  z2 argb2   argb3 argb2 ³ xmm2: inc3 inc2 inc1 inc0
				//edx:  j     ³ mm3:  z3 argb3               ³ xmm3:  r3   r2   r1   r0
				//esi:  -     ³ mm4:              z1    z0   ³ xmm4:            z3   z2
				//edi:scroff  ³ mm5:              z3    z2   ³ xmm5:
				//ebp:  -     ³ mm6: plc1 plc0               ³ xmm6:
				//esp:  -     ³ mm7: inc1 inc0               ³ xmm7:  z3   z2   z1   z0
	
			"movq	%mm7,mm7bak\n"
	"beg4h:\n\t"
			"pextrw	%mm6,1,%eax\n\t"
			"mov	angstart(,%eax,4),%eax\n\t"
			"movq	(%eax,%edx,8),%mm4\n\t"
			"pextrw	%mm6,3,%eax\n\t"
			"mov	angstart(,%eax,4),%eax\n\t"
			"movq	(%eax,%edx,8),%mm1\n\t"
			"paddd	mm7bak,%mm6\n\t"
			"pextrw	%mm6,1,%eax\n\t"
			"mov	angstart(,%eax,4),%eax\n\t"
			"movq	(%eax,%edx,8),%mm5\n\t"
			"pextrw	%mm6,3,%eax\n\t"
			"mov	angstart(,%eax,4),%eax\n\t"
			"movq	(%eax,%edx,8),%mm3\n\t"
			"paddd	mm7bak,%mm6\n\t"
	
			"movq	%mm4,%mm0\n\t"
			"movq	%mm5,%mm2\n\t"
	
					//Do Z
			"punpckhdq	%mm1,%mm4\n\t"
			"punpckhdq	%mm3,%mm5\n\t"
			"cvtpi2ps	%mm4,%xmm7\n\t"
			"cvtpi2ps	%mm5,%xmm4\n\t"
			"rsqrtps	%xmm0,%xmm3\n\t"
			"movlhps	%xmm4,%xmm7\n\t"
			"mulps	%xmm3,%xmm7\n\t"
			"movntps	%xmm7,(%edi,%ecx)\n\t"
			"addps	%xmm2,%xmm0\n\t"
			"addps	%xmm1,%xmm2\n\t"
	
					//Do colors
					//mm4:dist1 dist0
					//mm5:dist3 dist2
			"pxor	%mm7,%mm7\n\t"
			"punpcklbw	%mm7,%mm0\n\t"
			"punpcklbw	%mm7,%mm1\n\t"
			"punpcklbw	%mm7,%mm2\n\t"
			"punpcklbw	%mm7,%mm3\n\t"
	
			"movq	fogcol,%mm7\n\t"
			"psubw	%mm0,%mm7\n\t"
			"paddw	%mm7,%mm7\n\t"
			"pextrw	%mm4,1,%eax\n\t"
			"shr	$4,%eax\n\t"
			"pmulhw	foglut(,%eax,8),%mm7\n\t"
			"paddw	%mm7,%mm0\n\t"
	
			"movq	fogcol,%mm7\n\t"
			"psubw	%mm1,%mm7\n\t"
			"paddw	%mm7,%mm7\n\t"
			"pextrw	%mm4,3,%eax\n\t"
			"shr	$4,%eax\n\t"
			"pmulhw	foglut(,%eax,8),%mm7\n\t"
			"paddw	%mm7,%mm1\n\t"
	
			"movq	fogcol,%mm7\n\t"
			"psubw	%mm2,%mm7\n\t"
			"paddw	%mm7,%mm7\n\t"
			"pextrw	%mm5,1,%eax\n\t"
			"shr	$4,%eax\n\t"
			"pmulhw	foglut(,%eax,8),%mm7\n\t"
			"paddw	%mm7,%mm2\n\t"
	
			"movq	fogcol,%mm7\n\t"
			"psubw	%mm3,%mm7\n\t"
			"paddw	%mm7,%mm7\n\t"
			"pextrw	%mm5,3,%eax\n\t"
			"shr	$4,%eax\n\t"
			"pmulhw	foglut(,%eax,8),%mm7\n\t"
			"paddw	%mm7,%mm3\n\t"
	
			"packuswb	%mm1,%mm0\n\t"
			"packuswb	%mm3,%mm2\n\t"
			"movntq	%mm0,(%edi)\n\t"
			"movntq	%mm2,8(%edi)\n\t"
	
			"add	$16,%edi\n\t"
			"cmp	%esi,%edi\n\t"
			"jbe	beg4h\n\t"
			"add	$16,%esi\n\t"
			"cmp	%esi,%edi\n\t"
			"jae	endh\n\t"
	
			"movq	mm7bak,%mm7\n\t"
			"psrad	$1,%mm7\n"    //Restore mm7 from incr*2 to just incr for single loop
	"skip4h:\n"
	"beg1h:\n\t"
			"pextrw	%mm6,1,%eax\n\t"
			"paddd	%mm7,%mm6\n\t"
			"mov	angstart(,%eax,4),%eax\n\t"
	
					//Z
			"cvtsi2ss	4(%eax,%edx,8),%xmm7\n\t"
			"rsqrtss	%xmm0,%xmm3\n\t"
			"mulss	%xmm3,%xmm7\n\t"
			"shufps	%xmm0,0x39,%xmm0\n\t" //rotate right by 1
			"movss	%xmm7,(%edi,%ecx)\n\t"
	
					//Col
			"punpcklbw	(%eax,%edx,8),%mm0\n\t"
			"psrlw	$8,%mm0\n\t"
			"movq	fogcol,%mm1\n\t"
			"psubw	%mm0,%mm1\n\t"
			"paddw	%mm1,%mm1\n\t"
			"mov	4(%eax,%edx,8),%eax\n\t"
			"shr	16+4,%eax\n\t"
			"pmulhw	foglut(,%eax,8),%mm1\n\t"
			"paddw	%mm1,%mm0\n\t"
			"packuswb	%mm1,%mm0\n\t"
			"movl	%mm0,(%edi)\n\t"
	
			"add	$4,%edi\n\t"
			"cmp	%esi,%edi\n\t"
			"jb	beg1h\n"
	"endh:\n\t"
			"pop	%edi\n\t"
			"pop	%esi\n\t"
		);
		#endif
		#if defined(_MSC_VER) && !defined(__NOASM__) //MASM SYNTAX ASSEMBLY
		_asm
		{
			push	esi
			push	edi
	beghasm_p3:
			mov	eax, sx
			mov	ecx, sy
			mov	esi, p1
			mov	edx, ylookup[ecx*4]
			add	edx, frameplace
			lea	edi, [edx+eax*4]
			lea	esi, [edx+esi*4]
	
			and	eax, 0xfffffffc
			cvtsi2ss	xmm0, eax
			cvtsi2ss	xmm4, ecx
			movss	xmm1, xmm0
			movss	xmm5, xmm4
			mulss	xmm0, optistrx
			mulss	xmm1, optistry
			mulss	xmm4, optiheix
			mulss	xmm5, optiheiy
			addss	xmm0, optiaddx
			addss	xmm1, optiaddy
			addss	xmm0, xmm4
			addss	xmm1, xmm5
	
			mov	ecx, zbufoff
			mov	edx, j
			movd	mm6, plc
			movd	mm7, incr
	
			shufps	xmm0, xmm0, 0
			shufps	xmm1, xmm1, 0
			movaps	xmm2, opti4asm[2*16]
			movaps	xmm3, opti4asm[3*16]
			addps	xmm0, opti4asm[0*16]
			addps	xmm1, opti4asm[1*16]
				;xmm0	=  xmm0      ^2 +  xmm1      ^2        (p)
				;xmm2	= (xmm0+xmm2)^2 + (xmm1+xmm3)^2 - xmm0 (v)
				;xmm1	= ...                                  (a)
			addps	xmm2, xmm0  //This block converts inner loop...
			addps	xmm3, xmm1  //from: 1 / sqrt(x*x + y*y), x += xi, y += yi;
			mulps	xmm0, xmm0  //  to: 1 / sqrt(p), p += v, v += a;
			mulps	xmm1, xmm1
			mulps	xmm2, xmm2
			mulps	xmm3, xmm3
			addps	xmm0, xmm1
			movaps	xmm1, opti4asm[4*16]
			addps	xmm2, xmm3
			subps	xmm2, xmm0
	
				;Do	first 0-3 pixels to align unrolled loop of 4
			test	edi, 15
			jz	short skip1ha
	
			test	edi, 8
			jz	short skipshufa
			shufps	xmm0, xmm0, 0x4e ;rotate right by 2
	skipshufa:
			test	edi, 4
			jz	short skipshufb
			shufps	xmm0, xmm0, 0x39 ;rotate right by 1
	skipshufb:
	
	beg1ha:
			pextrw	eax, mm6, 1
			paddd	mm6, mm7
			mov	eax, angstart[eax*4]
	
				;Z
			cvtsi2ss	xmm7, [eax+edx*8+4]
			rsqrtss	xmm3, xmm0
			mulss	xmm7, xmm3
			shufps	xmm0, xmm0, 0x39 ;rotate right by 1
			movss	[edi+ecx], xmm7
	
				;Col
			punpcklbw	mm0, [eax+edx*8]
			psrlw	mm0, 8
			movq	mm1, fogcol
			psubw	mm1, mm0
			paddw	mm1, mm1
			mov	eax, [eax+edx*8+4]
			shr	eax, 16+4
			pmulhw	mm1, foglut[eax*8]
			paddw	mm0, mm1
			packuswb	mm0, mm1
			movd	[edi], mm0
	
			add	edi, 4
			cmp	edi, esi
			jz	short endh
			test	edi, 15
			jnz	short beg1ha
	
			addps	xmm0, xmm2
			addps	xmm2, xmm1
	skip1ha:
			lea	eax, [edi+16]      ;these 3 lines re-ordered
			cmp	eax, esi
			ja	short skip4h
	
			movq	mm0, mm6          ;mm0: 0,plc
			paddd	mm0, mm7         ;mm0: 0,plc+inc
			punpckldq	mm7, mm7     ;mm7: inc,inc
			punpckldq	mm6, mm0     ;mm6: plc+inc,plc
			paddd	mm7, mm7         ;mm7: inc+inc,inc+inc
	
			sub	esi, 16
	
				;eax: temp   ³ mm0:  z0 argb0   argb1 argb0 ³ xmm0: plc3 plc2 plc1 plc0
				;ebx:  -     ³ mm1:  z1 argb1               ³ xmm1: acc3 acc2 acc1 acc0
				;ecx:zbufoff ³ mm2:  z2 argb2   argb3 argb2 ³ xmm2: inc3 inc2 inc1 inc0
				;edx:  j     ³ mm3:  z3 argb3               ³ xmm3:  r3   r2   r1   r0
				;esi:  -     ³ mm4:              z1    z0   ³ xmm4:            z3   z2
				;edi:scroff  ³ mm5:              z3    z2   ³ xmm5:
				;ebp:  -     ³ mm6: plc1 plc0               ³ xmm6:
				;esp:  -     ³ mm7: inc1 inc0               ³ xmm7:  z3   z2   z1   z0
	
			movq	mm7bak, mm7
	beg4h:
			pextrw	eax, mm6, 1
			mov	eax, angstart[eax*4]
			movq	mm4, [eax+edx*8]
			pextrw	eax, mm6, 3
			mov	eax, angstart[eax*4]
			movq	mm1, [eax+edx*8]
			paddd	mm6, mm7bak
			pextrw	eax, mm6, 1
			mov	eax, angstart[eax*4]
			movq	mm5, [eax+edx*8]
			pextrw	eax, mm6, 3
			mov	eax, angstart[eax*4]
			movq	mm3, [eax+edx*8]
			paddd	mm6, mm7bak
	
			movq	mm0, mm4
			movq	mm2, mm5
	
				;Do	Z
			punpckhdq	mm4, mm1
			punpckhdq	mm5, mm3
			cvtpi2ps	xmm7, mm4
			cvtpi2ps	xmm4, mm5
			rsqrtps	xmm3, xmm0
			movlhps	xmm7, xmm4
			mulps	xmm7, xmm3
			movntps	[edi+ecx], xmm7
			addps	xmm0, xmm2
			addps	xmm2, xmm1
	
				;Do	colors
				;mm4:dist1	dist0
				;mm5:dist3	dist2
			pxor	mm7, mm7
			punpcklbw	mm0, mm7
			punpcklbw	mm1, mm7
			punpcklbw	mm2, mm7
			punpcklbw	mm3, mm7
	
			movq	mm7, fogcol
			psubw	mm7, mm0
			paddw	mm7, mm7
			pextrw	eax, mm4, 1
			shr	eax, 4
			pmulhw	mm7, foglut[eax*8]
			paddw	mm0, mm7
	
			movq	mm7, fogcol
			psubw	mm7, mm1
			paddw	mm7, mm7
			pextrw	eax, mm4, 3
			shr	eax, 4
			pmulhw	mm7, foglut[eax*8]
			paddw	mm1, mm7
	
			movq	mm7, fogcol
			psubw	mm7, mm2
			paddw	mm7, mm7
			pextrw	eax, mm5, 1
			shr	eax, 4
			pmulhw	mm7, foglut[eax*8]
			paddw	mm2, mm7
	
			movq	mm7, fogcol
			psubw	mm7, mm3
			paddw	mm7, mm7
			pextrw	eax, mm5, 3
			shr	eax, 4
			pmulhw	mm7, foglut[eax*8]
			paddw	mm3, mm7
	
			packuswb	mm0, mm1
			packuswb	mm2, mm3
			movntq	[edi], mm0
			movntq	[edi+8], mm2
	
			add	edi, 16
			cmp	edi, esi
			jbe	short beg4h
			add	esi, 16
			cmp	edi, esi
			jae	endh
	
			movq	mm7, mm7bak
			psrad	mm7, 1    ;Restore mm7 from incr*2 to just incr for single loop
	skip4h:
	beg1h:
			pextrw	eax, mm6, 1
			paddd	mm6, mm7
			mov	eax, angstart[eax*4]
	
				;Z
			cvtsi2ss	xmm7, [eax+edx*8+4]
			rsqrtss	xmm3, xmm0
			mulss	xmm7, xmm3
			shufps	xmm0, xmm0, 0x39 ;rotate right by 1
			movss	[edi+ecx], xmm7
	
				;Col
			punpcklbw	mm0, [eax+edx*8]
			psrlw	mm0, 8
			movq	mm1, fogcol
			psubw	mm1, mm0
			paddw	mm1, mm1
			mov	eax, [eax+edx*8+4]
			shr	eax, 16+4
			pmulhw	mm1, foglut[eax*8]
			paddw	mm0, mm1
			packuswb	mm0, mm1
			movd	[edi], mm0
	
			add	edi, 4
			cmp	edi, esi
			jb	short beg1h
	endh:
			pop	edi
			pop	esi
		}
		#endif
	}
	
	void vrendz (long sx, long sy, long p1, long iplc, long iinc)
	{
		#if defined(__GNUC__) && !defined(__NOASM__) //AT&T SYNTAX ASSEMBLY
		__asm__ __volatile__
		(
			"push	%ebx\n\t"
			"push	%esi\n\t"
			"push	%edi\n"
	"begvasm_p3:\n\t"
			"mov	sx,%esi\n\t"
			"mov	sy,%eax\n\t"
			"mov	p1,%edx\n\t"
			"mov	ylookup(,%eax,4),%ecx\n\t"
			"add	frameplace,%ecx\n\t"
			"lea	(%ecx,%edx,4),%edx\n\t"
			"lea	(%ecx,%esi,4),%edi\n\t"
	
			"mov	%esi,%ecx\n\t"
			"and	$0xfffffffc,%ecx\n\t"
			"cvtsi2ss	%ecx,%xmm0\n\t"
			"cvtsi2ss	%eax,%xmm4\n\t"
			"movss	%xmm0,%xmm1\n\t"
			"movss	%xmm4,%xmm5\n\t"
			"mulss	optistrx,%xmm0\n\t"
			"mulss	optistry,%xmm1\n\t"
			"mulss	optiheix,%xmm4\n\t"
			"mulss	optiheiy,%xmm5\n\t"
			"addss	optiaddx,%xmm0\n\t"
			"addss	optiaddy,%xmm1\n\t"
			"addss	%xmm4,%xmm0\n\t"
			"addss	%xmm5,%xmm1\n\t"
	
			"shufps	%xmm0,0,%xmm0\n\t"
			"shufps	%xmm1,0,%xmm1\n\t"
			"movaps	opti4asm+2*16,%xmm2\n\t"
			"movaps	opti4asm+3*16,%xmm3\n\t"
			"addps	opti4asm+0*16,%xmm0\n\t"
			"addps	opti4asm+1*16,%xmm1\n\t"
					//xmm0 =  xmm0      ^2 +  xmm1      ^2        (p)\n\t"
					//xmm2 = (xmm0+xmm2)^2 + (xmm1+xmm3)^2 - xmm0 (v)\n\t"
					//xmm1 = ...                                  (a)\n\t"
			"addps	%xmm0,%xmm2 \n\t" //This block converts inner loop...
			"addps	%xmm1,%xmm3 \n\t" //from: 1 / sqrt(x*x + y*y), x += xi, y += yi;
			"mulps	%xmm0,%xmm0 \n\t" //  to: 1 / sqrt(p), p += v, v += a;
			"mulps	%xmm1,%xmm1\n\t"
			"mulps	%xmm2,%xmm2\n\t"
			"mulps	%xmm3,%xmm3\n\t"
			"addps	%xmm1,%xmm0\n\t"
			"movaps	opti4asm+4*16,%xmm1\n\t"
			"addps	%xmm3,%xmm2\n\t"
			"subps	%xmm0,%xmm2\n\t"
	
			"mov	%edx,p1\n\t"
			"mov	zbufoff,%ecx\n\t"
			"shl	$2,%esi\n\t"
			"add	uurend,%esi\n\t"
			"mov	iplc,%ebx\n\t"
	
			"cmp	%edx,%edi\n\t"
			"jae	endv\n\t"
	
					//Do first 0-3 pixels to align unrolled loop of 4\n\t"
			"test	$15,%edi\n\t"
			"jz	skip1va\n\t"
	
			"test	$8,%edi\n\t"
			"jz	skipshufc\n\t"
			"shufps	%xmm0,0x4e,%xmm0\n" //rotate right by 2
	"skipshufc:\n\t"
			"test	$4,%edi\n\t"
			"jz	skipshufd\n\t"
			"shufps	%xmm0,0x39,%xmm0\n" //rotate right by 1
	"skipshufd:\n"
	
	"beg1va:\n\t"
			"mov	(%esi),%edx\n\t"
			"mov	MAXXDIM(,%esi,4),%eax\n\t"
			"add	%edx,%eax\n\t"
			"sar	$16,%edx\n\t"
			"mov	angstart(,%edx,4),%edx\n\t"
			"mov	%eax,(%esi)\n\t"
			"mov	(%edx,%ebx,8),%eax\n\t"
			"mov	%eax,(%edi)\n\t"
			"cvtsi2ss	4(%edx,%ebx,8),%xmm7\n\t"
			"rsqrtss	%xmm0,%xmm3\n\t"
			"mulss	%xmm3,%xmm7\n\t"
			"shufps	%xmm0,0x39,%xmm0\n\t" //rotate right by 1
			"movss	%xmm7,(%edi,%ecx)\n\t"
			"add	iinc,%ebx\n\t"
			"add	$4,%esi\n\t"
			"add	$4,%edi\n\t"
			"cmp	p1,%edi\n\t"
			"jz	endv\n\t"
			"test	$15,%edi\n\t"
			"jnz	beg1va\n\t"
	
			"addps	%xmm2,%xmm0\n\t"
			"addps	%xmm1,%xmm2\n"
	"skip1va:\n\t"
			"lea	16(%edi),%edx\n\t"
			"cmp	p1,%edx\n\t"
			"ja	prebeg1v\n\t"
	
			"cmp	$0,iinc\n\t"
			"jl	beg4vn\n"
	
	"beg4vp:\n\t"
			"movq	(%esi),%mm6\n\t"
			"movq	8(%esi),%mm7\n\t"
			"pextrw	%mm6,1,%eax\n\t"
			"pextrw	%mm6,3,%edx\n\t"
			"paddd	MAXXDIM(,%esi,4),%mm6\n\t"
			"mov	angstart(,%eax,4),%eax\n\t"
			"mov	angstart(,%edx,4),%edx\n\t"
			"movq	(%eax,%ebx,8),%mm0\n\t"
			"movq	8(%edx,%ebx,8),%mm1\n\t"
			"pextrw	%mm7,1,%eax\n\t"
			"pextrw	%mm7,3,%edx\n\t"
			"paddd	8+MAXXDIM(,%esi,4),%mm7\n\t"
			"mov	angstart(,%eax,4),%eax\n\t"
			"mov	angstart(,%edx,4),%edx\n\t"
			"movq	16(%eax,%ebx,8),%mm2\n\t"
			"movq	24(%edx,%ebx,8),%mm3\n\t"
			"add	$4,%ebx\n\t"
	
			"movq	%mm0,%mm4\n\t"
			"movq	%mm2,%mm5\n\t"
			"punpckldq	%mm1,%mm0\n\t"
			"punpckldq	%mm3,%mm2\n\t"
			"movntq	%mm0,(%edi)\n\t"
			"movntq	%mm2,8(%edi)\n\t"
	
			"punpckhdq	%mm1,%mm4\n\t"
			"punpckhdq	%mm3,%mm5\n\t"
			"cvtpi2ps	%mm4,%xmm7\n\t"
			"cvtpi2ps	%mm5,%xmm4\n\t"
			"rsqrtps	%xmm0,%xmm3\n\t"
			"movlhps	%xmm4,%xmm7\n\t"
			"mulps	%xmm3,%xmm7\n\t"
			"movntps	%xmm7,(%edi,%ecx)\n\t"
			"addps	%xmm2,%xmm0\n\t"
			"addps	%xmm1,%xmm2\n\t"
	
			"movq	%mm6,(%esi)\n\t"
			"movq	%mm7,8(%esi)\n\t"
	
			"add	$16,%esi\n\t"
			"add	$16,%edi\n\t"
			"lea	16(%edi),%edx\n\t"
			"cmp	p1,%edx\n\t"
			"jbe	beg4vp\n\t"
			"cmp	p1,%edi\n\t"
			"jae	endv\n\t"
			"jmp	prebeg1v\n"
	
	"beg4vn:\n\t"
			"movq	(%esi),%mm6\n\t"
			"movq	8(%esi),%mm7\n\t"
			"pextrw	%mm6,1,%eax\n\t"
			"pextrw	%mm6,3,%edx\n\t"
			"paddd	MAXXDIM(,%esi,4),%mm6\n\t"
			"mov	angstart(,%eax,4),%eax\n\t"
			"mov	angstart(,%edx,4),%edx\n\t"
			"movq	(%eax,%ebx,8),%mm0\n\t"
			"movq	-8(%edx,%ebx,8),%mm1\n\t"
			"pextrw	%mm7,1,%eax\n\t"
			"pextrw	%mm7,3,%edx\n\t"
			"paddd	8+MAXXDIM(,%esi,4),%mm7\n\t"
			"mov	angstart(,%eax,4),%eax\n\t"
			"mov	angstart(,%edx,4),%edx\n\t"
			"movq	-16(%eax,%ebx,8),%mm2\n\t"
			"movq	-24(%edx,%ebx,8),%mm3\n\t"
			"sub	$4,%ebx\n\t"
	
			"movq	%mm0,%mm4\n\t"
			"movq	%mm2,%mm5\n\t"
			"punpckldq	%mm1,%mm0\n\t"
			"punpckldq	%mm3,%mm2\n\t"
			"movntq	%mm0,(%edi)\n\t"
			"movntq	%mm2,8(%edi)\n\t"
	
			"punpckhdq	%mm1,%mm4\n\t"
			"punpckhdq	%mm3,%mm5\n\t"
			"cvtpi2ps	%mm4,%xmm7\n\t"
			"cvtpi2ps	%mm5,%xmm4\n\t"
			"rsqrtps	%xmm0,%xmm3\n\t"
			"movlhps	%xmm4,%xmm7\n\t"
			"mulps	%xmm3,%xmm7\n\t"
			"movntps	%xmm7,(%edi,%ecx)\n\t"
			"addps	%xmm2,%xmm0\n\t"
			"addps	%xmm1,%xmm2\n\t"
	
			"movq	%mm6,(%esi)\n\t"
			"movq	%mm7,8(%esi)\n\t"
	
			"add	$16,%esi\n\t"
			"add	$16,%edi\n\t"
			"lea	16(%edi),%edx\n\t"
			"cmp	p1,%edx\n\t"
			"jbe	beg4vn\n\t"
			"cmp	p1,%edi\n\t"
			"jae	endv\n"
	
	"prebeg1v:\n"
	"beg1v:\n\t"
			"mov	(%esi),%edx\n\t"
			"mov	MAXXDIM(,%esi,4),%eax\n\t"
			"add	%edx,%eax\n\t"
			"sar	$16,%edx\n\t"
			"mov	angstart(,%edx,4),%edx\n\t"
			"mov	%eax,(%esi)\n\t"
			"mov	(%edx,%ebx,8),%eax\n\t"
			"mov	%eax,(%edi)\n\t"
			"cvtsi2ss	4(%edx,%ebx,8),%xmm7\n\t"
			"rsqrtss	%xmm0,%xmm3\n\t"
			"mulss	%xmm3,%xmm7\n\t"
			"shufps	%xmm0,0x39,%xmm0\n\t" //rotate right by 1
			"movss	%xmm7,(%edi,%ecx)\n\t"
			"add	iinc,%ebx\n\t"
			"add	$4,%esi\n\t"
			"add	$4,%edi\n\t"
			"cmp	p1,%edi\n\t"
			"jne	beg1v\n"
	"endv:\n\t"
			"pop	%edi\n\t"
			"pop	%esi\n\t"
			"pop	%ebx\n\t"
		);
		#endif
		#if defined(_MSC_VER) && !defined(__NOASM__) //MASM SYNTAX ASSEMBLY
		_asm
		{
			push	ebx
			push	esi
			push	edi
	begvasm_p3:
			mov	esi, sx
			mov	eax, sy
			mov	edx, p1
			mov	ecx, ylookup[eax*4]
			add	ecx, frameplace
			lea	edx, [ecx+edx*4]
			lea	edi, [ecx+esi*4]
	
			mov	ecx, esi
			and	ecx, 0xfffffffc
			cvtsi2ss	xmm0, ecx
			cvtsi2ss	xmm4, eax
			movss	xmm1, xmm0
			movss	xmm5, xmm4
			mulss	xmm0, optistrx
			mulss	xmm1, optistry
			mulss	xmm4, optiheix
			mulss	xmm5, optiheiy
			addss	xmm0, optiaddx
			addss	xmm1, optiaddy
			addss	xmm0, xmm4
			addss	xmm1, xmm5
	
			shufps	xmm0, xmm0, 0
			shufps	xmm1, xmm1, 0
			movaps	xmm2, opti4asm[2*16]
			movaps	xmm3, opti4asm[3*16]
			addps	xmm0, opti4asm[0*16]
			addps	xmm1, opti4asm[1*16]
				;xmm0	=  xmm0      ^2 +  xmm1      ^2        (p)
				;xmm2	= (xmm0+xmm2)^2 + (xmm1+xmm3)^2 - xmm0 (v)
				;xmm1	= ...                                  (a)
			addps	xmm2, xmm0  ;This block converts inner loop...
			addps	xmm3, xmm1  ;from: 1 / sqrt(x*x + y*y), x += xi, y += yi;
			mulps	xmm0, xmm0  ;  to: 1 / sqrt(p), p += v, v += a;
			mulps	xmm1, xmm1
			mulps	xmm2, xmm2
			mulps	xmm3, xmm3
			addps	xmm0, xmm1
			movaps	xmm1, opti4asm[4*16]
			addps	xmm2, xmm3
			subps	xmm2, xmm0
	
			mov	p1, edx
			mov	ecx, zbufoff
			shl	esi, 2
			add	esi, uurend
			mov	ebx, iplc
	
			cmp	edi, edx
			jae	short endv
	
				;Do	first 0-3 pixels to align unrolled loop of 4
			test	edi, 15
			jz	short skip1va
	
			test	edi, 8
			jz	short skipshufc
			shufps	xmm0, xmm0, 0x4e ;rotate right by 2
	skipshufc:
			test	edi, 4
			jz	short skipshufd
			shufps	xmm0, xmm0, 0x39 ;rotate right by 1
	skipshufd:
	
	beg1va:
			mov	edx, [esi]
			mov	eax, [esi+MAXXDIM*4]
			add	eax, edx
			sar	edx, 16
			mov	edx, angstart[edx*4]
			mov	[esi], eax
			mov	eax, [edx+ebx*8]
			mov	[edi], eax
			cvtsi2ss	xmm7, [edx+ebx*8+4]
			rsqrtss	xmm3, xmm0
			mulss	xmm7, xmm3
			shufps	xmm0, xmm0, 0x39 ;rotate right by 1
			movss	[edi+ecx], xmm7
			add	ebx, iinc
			add	esi, 4
			add	edi, 4
			cmp	edi, p1
			jz	short endv
			test	edi, 15
			jnz	short beg1va
	
			addps	xmm0, xmm2
			addps	xmm2, xmm1
	skip1va:
			lea	edx, [edi+16]
			cmp	edx, p1
			ja	short prebeg1v
	
			cmp	iinc, 0
			jl	short beg4vn
	
	beg4vp:
			movq	mm6, [esi]
			movq	mm7, [esi+8]
			pextrw	eax, mm6, 1
			pextrw	edx, mm6, 3
			paddd	mm6, [esi+MAXXDIM*4]
			mov	eax, angstart[eax*4]
			mov	edx, angstart[edx*4]
			movq	mm0, [eax+ebx*8]
			movq	mm1, [edx+ebx*8+8]
			pextrw	eax, mm7, 1
			pextrw	edx, mm7, 3
			paddd	mm7, [esi+8+MAXXDIM*4]
			mov	eax, angstart[eax*4]
			mov	edx, angstart[edx*4]
			movq	mm2, [eax+ebx*8+16]
			movq	mm3, [edx+ebx*8+24]
			add	ebx, 4
	
			movq	mm4, mm0
			movq	mm5, mm2
			punpckldq	mm0, mm1
			punpckldq	mm2, mm3
			movntq	[edi], mm0
			movntq	[edi+8], mm2
	
			punpckhdq	mm4, mm1
			punpckhdq	mm5, mm3
			cvtpi2ps	xmm7, mm4
			cvtpi2ps	xmm4, mm5
			rsqrtps	xmm3, xmm0
			movlhps	xmm7, xmm4
			mulps	xmm7, xmm3
			movntps	[edi+ecx], xmm7
			addps	xmm0, xmm2
			addps	xmm2, xmm1
	
			movq	[esi], mm6
			movq	[esi+8], mm7
	
			add	esi, 16
			add	edi, 16
			lea	edx, [edi+16]
			cmp	edx, p1
			jbe	short beg4vp
			cmp	edi, p1
			jae	short endv
			jmp	short prebeg1v
	
	beg4vn:
			movq	mm6, [esi]
			movq	mm7, [esi+8]
			pextrw	eax, mm6, 1
			pextrw	edx, mm6, 3
			paddd	mm6, [esi+MAXXDIM*4]
			mov	eax, angstart[eax*4]
			mov	edx, angstart[edx*4]
			movq	mm0, [eax+ebx*8]
			movq	mm1, [edx+ebx*8-8]
			pextrw	eax, mm7, 1
			pextrw	edx, mm7, 3
			paddd	mm7, [esi+8+MAXXDIM*4]
			mov	eax, angstart[eax*4]
			mov	edx, angstart[edx*4]
			movq	mm2, [eax+ebx*8-16]
			movq	mm3, [edx+ebx*8-24]
			sub	ebx, 4
	
			movq	mm4, mm0
			movq	mm5, mm2
			punpckldq	mm0, mm1
			punpckldq	mm2, mm3
			movntq	[edi], mm0
			movntq	[edi+8], mm2
	
			punpckhdq	mm4, mm1
			punpckhdq	mm5, mm3
			cvtpi2ps	xmm7, mm4
			cvtpi2ps	xmm4, mm5
			rsqrtps	xmm3, xmm0
			movlhps	xmm7, xmm4
			mulps	xmm7, xmm3
			movntps	[edi+ecx], xmm7
			addps	xmm0, xmm2
			addps	xmm2, xmm1
	
			movq	[esi], mm6
			movq	[esi+8], mm7
	
			add	esi, 16
			add	edi, 16
			lea	edx, [edi+16]
			cmp	edx, p1
			jbe	short beg4vn
			cmp	edi, p1
			jae	short endv
	
	prebeg1v:
	beg1v:
			mov	edx, [esi]
			mov	eax, [esi+MAXXDIM*4]
			add	eax, edx
			sar	edx, 16
			mov	edx, angstart[edx*4]
			mov	[esi], eax
			mov	eax, [edx+ebx*8]
			mov	[edi], eax
			cvtsi2ss	xmm7, [edx+ebx*8+4]
			rsqrtss	xmm3, xmm0
			mulss	xmm7, xmm3
			shufps	xmm0, xmm0, 0x39 ;rotate right by 1
			movss	[edi+ecx], xmm7
			add	ebx, iinc
			add	esi, 4
			add	edi, 4
			cmp	edi, p1
			jne	short beg1v
	endv:
			pop	edi
			pop	esi
			pop	ebx
		}
		#endif
	}
	
	void vrendzfog (long sx, long sy, long p1, long iplc, long iinc)
	{
		#if defined(__GNUC__) && !defined(__NOASM__) //AT&T SYNTAX ASSEMBLY
		__asm__ __volatile__
		(
			"push	%ebx\n\t"
			"push	%esi\n\t"
			"push	%edi\n"
	"begvasm_p3:\n\t"
			"mov	sx,%esi\n\t"
			"mov	sy,%eax\n\t"
			"mov	p1,%edx\n\t"
			"mov	ylookup(,%eax,4),%ecx\n\t"
			"add	frameplace,%ecx\n\t"
			"lea	(%ecx,%edx,4),%edx\n\t"
			"lea	(%ecx,%esi,4),%edi\n\t"
	
			"mov	%esi,%ecx\n\t"
			"and	$0xfffffffc,%ecx\n\t"
			"cvtsi2ss	%ecx,%xmm0\n\t"
			"cvtsi2ss	%eax,%xmm4\n\t"
			"movss	%xmm0,%xmm1\n\t"
			"movss	%xmm4,%xmm5\n\t"
			"mulss	optistrx,%xmm0\n\t"
			"mulss	optistry,%xmm1\n\t"
			"mulss	optiheix,%xmm4\n\t"
			"mulss	optiheiy,%xmm5\n\t"
			"addss	optiaddx,%xmm0\n\t"
			"addss	optiaddy,%xmm1\n\t"
			"addss	%xmm4,%xmm0\n\t"
			"addss	%xmm5,%xmm1\n\t"
	
			"shufps	%xmm0,0,%xmm0\n\t"
			"shufps	%xmm1,0,%xmm1\n\t"
			"movaps	opti4asm+2*16,%xmm2\n\t"
			"movaps	opti4asm+3*16,%xmm3\n\t"
			"addps	opti4asm+0*16,%xmm0\n\t"
			"addps	opti4asm+1*16,%xmm1\n\t"
					//xmm0 =  xmm0      ^2 +  xmm1      ^2        (p)\n\t"
					//xmm2 = (xmm0+xmm2)^2 + (xmm1+xmm3)^2 - xmm0 (v)\n\t"
					//xmm1 = ...                                  (a)\n\t"
			"addps	%xmm0,%xmm2 \n\t" //This block converts inner loop...
			"addps	%xmm1,%xmm3 \n\t" //from: 1 / sqrt(x*x + y*y), x += xi, y += yi;
			"mulps	%xmm0,%xmm0 \n\t" //  to: 1 / sqrt(p), p += v, v += a;
			"mulps	%xmm1,%xmm1\n\t"
			"mulps	%xmm2,%xmm2\n\t"
			"mulps	%xmm3,%xmm3\n\t"
			"addps	%xmm1,%xmm0\n\t"
			"movaps	opti4asm+4*16,%xmm1\n\t"
			"addps	%xmm3,%xmm2\n\t"
			"subps	%xmm0,%xmm2\n\t"
	
			"mov	%edx,p1\n\t"
			"mov	zbufoff,%ecx\n\t"
			"shl	$2,%esi\n\t"
			"add	uurend,%esi\n\t"
			"mov	iplc,%ebx\n\t"
	
			"cmp	%edx,%edi\n\t"
			"jae	endv\n\t"
	
					//Do first 0-3 pixels to align unrolled loop of 4\n\t"
			"test	$15,%edi\n\t"
			"jz	skip1va\n\t"
	
			"test	$8,%edi\n\t"
			"jz	skipshufc\n\t"
			"shufps	%xmm0,0x4e,%xmm0\n" //rotate right by 2
	"skipshufc:\n\t"
			"test	$4,%edi\n\t"
			"jz	skipshufd\n\t"
			"shufps	%xmm0,0x39,%xmm0\n" //rotate right by 1
	"skipshufd:\n"
	
	"beg1va:\n\t"
			"mov	(%esi),%edx\n\t"
			"mov	MAXXDIM(,%esi,4),%eax\n\t"
			"add	%edx,%eax\n\t"
			"sar	$16,%edx\n\t"
			"mov	angstart(,%edx,4),%edx\n\t"
			"mov	%eax,(%esi)\n\t"
	
					//Z\n\t"
			"cvtsi2ss	4(%edx,%ebx,8),%xmm7\n\t"
			"rsqrtss	%xmm0,%xmm3\n\t"
			"mulss	%xmm3,%xmm7\n\t"
			"shufps	%xmm0,0x39,%xmm0\n\t" //rotate right by 1
			"movss	%xmm7,(%edi,%ecx)\n\t"
	
					//Col\n\t"
			"punpcklbw	(%edx,%ebx,8),%mm0\n\t"
			"psrlw	$8,%mm0\n\t"
			"movq	fogcol,%mm1\n\t"
			"psubw	%mm0,%mm1\n\t"
			"paddw	%mm1,%mm1\n\t"
			"mov	4(%edx,%ebx,8),%eax\n\t"
			"shr	16+4,%eax\n\t"
			"pmulhw	foglut(,%eax,8),%mm1\n\t"
			"paddw	%mm1,%mm0\n\t"
			"packuswb	%mm1,%mm0\n\t"
			"movl	%mm0,(%edi)\n\t"
	
			"add	iinc,%ebx\n\t"
			"add	$4,%esi\n\t"
			"add	$4,%edi\n\t"
			"cmp	p1,%edi\n\t"
			"jz	endv\n\t"
			"test	$15,%edi\n\t"
			"jnz	beg1va\n\t"
	
			"addps	%xmm2,%xmm0\n\t"
			"addps	%xmm1,%xmm2\n"
	"skip1va:\n\t"
			"lea	16(%edi),%edx\n\t"
			"cmp	p1,%edx\n\t"
			"ja	prebeg1v\n\t"
	
			"cmp	$0,iinc\n\t"
			"jl	beg4vn\n"
	
	"beg4vp:\n\t"
			"movq	(%esi),%mm6\n\t"
			"movq	8(%esi),%mm7\n\t"
			"pextrw	%mm6,1,%eax\n\t"
			"pextrw	%mm6,3,%edx\n\t"
			"paddd	MAXXDIM(,%esi,4),%mm6\n\t"
			"mov	angstart(,%eax,4),%eax\n\t"
			"mov	angstart(,%edx,4),%edx\n\t"
			"movq	(%eax,%ebx,8),%mm4\n\t"
			"movq	8(%edx,%ebx,8),%mm1\n\t"
			"pextrw	%mm7,1,%eax\n\t"
			"pextrw	%mm7,3,%edx\n\t"
			"paddd	8+MAXXDIM(,%esi,4),%mm7\n\t"
			"mov	angstart(,%eax,4),%eax\n\t"
			"mov	angstart(,%edx,4),%edx\n\t"
			"movq	16(%eax,%ebx,8),%mm5\n\t"
			"movq	24(%edx,%ebx,8),%mm3\n\t"
			"add	$4,%ebx\n\t"
	
					//Do Z\n\t"
			"movq	%mm4,%mm0\n\t"
			"movq	%mm5,%mm2\n\t"
			"punpckhdq	%mm1,%mm4\n\t"
			"punpckhdq	%mm3,%mm5\n\t"
			"cvtpi2ps	%mm4,%xmm7\n\t"
			"cvtpi2ps	%mm5,%xmm4\n\t"
			"rsqrtps	%xmm0,%xmm3\n\t"
			"movlhps	%xmm4,%xmm7\n\t"
			"mulps	%xmm3,%xmm7\n\t"
			"movntps	%xmm7,(%edi,%ecx)\n\t"
			"addps	%xmm2,%xmm0\n\t"
			"addps	%xmm1,%xmm2\n\t"
	
			"movq	%mm6,(%esi)\n\t"
			"movq	%mm7,8(%esi)\n\t"
	
					//Do color\n\t"
			"pxor	%mm7,%mm7\n\t"
			"punpcklbw	%mm7,%mm0\n\t"
			"punpcklbw	%mm7,%mm1\n\t"
			"punpcklbw	%mm7,%mm2\n\t"
			"punpcklbw	%mm7,%mm3\n\t"
	
			"movq	fogcol,%mm7\n\t"
			"psubw	%mm0,%mm7\n\t"
			"paddw	%mm7,%mm7\n\t"
			"pextrw	%mm4,1,%eax\n\t"
			"shr	$4,%eax\n\t"
			"pmulhw	foglut(,%eax,8),%mm7\n\t"
			"paddw	%mm7,%mm0\n\t"
	
			"movq	fogcol,%mm7\n\t"
			"psubw	%mm1,%mm7\n\t"
			"paddw	%mm7,%mm7\n\t"
			"pextrw	%mm4,3,%eax\n\t"
			"shr	$4,%eax\n\t"
			"pmulhw	foglut(,%eax,8),%mm7\n\t"
			"paddw	%mm7,%mm1\n\t"
	
			"movq	fogcol,%mm7\n\t"
			"psubw	%mm2,%mm7\n\t"
			"paddw	%mm7,%mm7\n\t"
			"pextrw	%mm5,1,%eax\n\t"
			"shr	$4,%eax\n\t"
			"pmulhw	foglut(,%eax,8),%mm7\n\t"
			"paddw	%mm7,%mm2\n\t"
	
			"movq	fogcol,%mm7\n\t"
			"psubw	%mm3,%mm7\n\t"
			"paddw	%mm7,%mm7\n\t"
			"pextrw	%mm5,3,%eax\n\t"
			"shr	$4,%eax\n\t"
			"pmulhw	foglut(,%eax,8),%mm7\n\t"
			"paddw	%mm7,%mm3\n\t"
	
			"packuswb	%mm1,%mm0\n\t"
			"packuswb	%mm3,%mm2\n\t"
			"movntq	%mm0,(%edi)\n\t"
			"movntq	%mm2,8(%edi)\n\t"
	
			"add	$16,%esi\n\t"
			"add	$16,%edi\n\t"
			"lea	16(%edi),%edx\n\t"
			"cmp	p1,%edx\n\t"
			"jbe	beg4vp\n\t"
			"cmp	p1,%edi\n\t"
			"jae	endv\n\t"
			"jmp	prebeg1v\n"
	
	"beg4vn:\n\t"
			"movq	(%esi),%mm6\n\t"
			"movq	8(%esi),%mm7\n\t"
			"pextrw	%mm6,1,%eax\n\t"
			"pextrw	%mm6,3,%edx\n\t"
			"paddd	MAXXDIM(,%esi,4),%mm6\n\t"
			"mov	angstart(,%eax,4),%eax\n\t"
			"mov	angstart(,%edx,4),%edx\n\t"
			"movq	(%eax,%ebx,8),%mm4\n\t"
			"movq	-8(%edx,%ebx,8),%mm1\n\t"
			"pextrw	%mm7,1,%eax\n\t"
			"pextrw	%mm7,3,%edx\n\t"
			"paddd	8+MAXXDIM(,%esi,4),%mm7\n\t"
			"mov	angstart(,%eax,4),%eax\n\t"
			"mov	angstart(,%edx,4),%edx\n\t"
			"movq	-16(%eax,%ebx,8),%mm5\n\t"
			"movq	-24(%edx,%ebx,8),%mm3\n\t"
			"sub	$4,%ebx\n\t"
	
					//Do Z\n\t"
			"movq	%mm4,%mm0\n\t"
			"movq	%mm5,%mm2\n\t"
			"punpckhdq	%mm1,%mm4\n\t"
			"punpckhdq	%mm3,%mm5\n\t"
			"cvtpi2ps	%mm4,%xmm7\n\t"
			"cvtpi2ps	%mm5,%xmm4\n\t"
			"rsqrtps	%xmm0,%xmm3\n\t"
			"movlhps	%xmm4,%xmm7\n\t"
			"mulps	%xmm3,%xmm7\n\t"
			"movntps	%xmm7,(%edi,%ecx)\n\t"
			"addps	%xmm2,%xmm0\n\t"
			"addps	%xmm1,%xmm2\n\t"
	
			"movq	%mm6,(%esi)\n\t"
			"movq	%mm7,8(%esi)\n\t"
	
					//Do color\n\t"
			"pxor	%mm7,%mm7\n\t"
			"punpcklbw	%mm7,%mm0\n\t"
			"punpcklbw	%mm7,%mm1\n\t"
			"punpcklbw	%mm7,%mm2\n\t"
			"punpcklbw	%mm7,%mm3\n\t"
	
			"movq	fogcol,%mm7\n\t"
			"psubw	%mm0,%mm7\n\t"
			"paddw	%mm7,%mm7\n\t"
			"pextrw	%mm4,1,%eax\n\t"
			"shr	$4,%eax\n\t"
			"pmulhw	foglut(,%eax,8),%mm7\n\t"
			"paddw	%mm7,%mm0\n\t"
	
			"movq	fogcol,%mm7\n\t"
			"psubw	%mm1,%mm7\n\t"
			"paddw	%mm7,%mm7\n\t"
			"pextrw	%mm4,3,%eax\n\t"
			"shr	$4,%eax\n\t"
			"pmulhw	foglut(,%eax,8),%mm7\n\t"
			"paddw	%mm7,%mm1\n\t"
	
			"movq	fogcol,%mm7\n\t"
			"psubw	%mm2,%mm7\n\t"
			"paddw	%mm7,%mm7\n\t"
			"pextrw	%mm5,1,%eax\n\t"
			"shr	$4,%eax\n\t"
			"pmulhw	foglut(,%eax,8),%mm7\n\t"
			"paddw	%mm7,%mm2\n\t"
	
			"movq	fogcol,%mm7\n\t"
			"psubw	%mm3,%mm7\n\t"
			"paddw	%mm7,%mm7\n\t"
			"pextrw	%mm5,3,%eax\n\t"
			"shr	$4,%eax\n\t"
			"pmulhw	foglut(,%eax,8),%mm7\n\t"
			"paddw	%mm7,%mm3\n\t"
	
			"packuswb	%mm1,%mm0\n\t"
			"packuswb	%mm3,%mm2\n\t"
			"movntq	%mm0,(%edi)\n\t"
			"movntq	%mm2,8(%edi)\n\t"
	
			"add	$16,%esi\n\t"
			"add	$16,%edi\n\t"
			"lea	16(%edi),%edx\n\t"
			"cmp	p1,%edx\n\t"
			"jbe	beg4vn\n\t"
			"cmp	p1,%edi\n\t"
			"jae	endv\n"
	
	"prebeg1v:\n"
	"beg1v:\n\t"
			"mov	(%esi),%edx\n\t"
			"mov	MAXXDIM(,%esi,4),%eax\n\t"
			"add	%edx,%eax\n\t"
			"sar	$16,%edx\n\t"
			"mov	angstart(,%edx,4),%edx\n\t"
			"mov	%eax,(%esi)\n\t"
	
					//Z\n\t"
			"cvtsi2ss	4(%edx,%ebx,8),%xmm7\n\t"
			"rsqrtss	%xmm0,%xmm3\n\t"
			"mulss	%xmm3,%xmm7\n\t"
			"shufps	%xmm0,0x39,%xmm0\n\t" //rotate right by 1
			"movss	%xmm7,(%edi,%ecx)\n\t"
	
					//Col\n\t"
			"punpcklbw	(%edx,%ebx,8),%mm0\n\t"
			"psrlw	$8,%mm0\n\t"
			"movq	fogcol,%mm1\n\t"
			"psubw	%mm0,%mm1\n\t"
			"paddw	%mm1,%mm1\n\t"
			"mov	4(%edx,%ebx,8),%eax\n\t"
			"shr	16+4,%eax\n\t"
			"pmulhw	foglut(,%eax,8),%mm1\n\t"
			"paddw	%mm1,%mm0\n\t"
			"packuswb	%mm1,%mm0\n\t"
			"movl	%mm0,(%edi)\n\t"
	
			"add	iinc,%ebx\n\t"
			"add	$4,%esi\n\t"
			"add	$4,%edi\n\t"
			"cmp	p1,%edi\n\t"
			"jne	beg1v\n"
	"endv:\n\t"
			"pop	%edi\n\t"
			"pop	%esi\n\t"
			"pop	%ebx\n\t"
		);
		#endif
		#if defined(_MSC_VER) && !defined(__NOASM__) //MASM SYNTAX ASSEMBLY
	_asm
	{
		push	ebx
		push	esi
		push	edi
begvasm_p3:
		mov	esi, sx
		mov	eax, sy
		mov	edx, p1
		mov	ecx, ylookup[eax*4]
		add	ecx, frameplace
		lea	edx, [ecx+edx*4]
		lea	edi, [ecx+esi*4]

		mov	ecx, esi
		and	ecx, 0xfffffffc
		cvtsi2ss	xmm0, ecx
		cvtsi2ss	xmm4, eax
		movss	xmm1, xmm0
		movss	xmm5, xmm4
		mulss	xmm0, optistrx
		mulss	xmm1, optistry
		mulss	xmm4, optiheix
		mulss	xmm5, optiheiy
		addss	xmm0, optiaddx
		addss	xmm1, optiaddy
		addss	xmm0, xmm4
		addss	xmm1, xmm5

		shufps	xmm0, xmm0, 0
		shufps	xmm1, xmm1, 0
		movaps	xmm2, opti4asm[2*16]
		movaps	xmm3, opti4asm[3*16]
		addps	xmm0, opti4asm[0*16]
		addps	xmm1, opti4asm[1*16]
			;xmm0	=  xmm0      ^2 +  xmm1      ^2        (p)
			;xmm2	= (xmm0+xmm2)^2 + (xmm1+xmm3)^2 - xmm0 (v)
			;xmm1	= ...                                  (a)
		addps	xmm2, xmm0  ;This block converts inner loop...
		addps	xmm3, xmm1  ;from: 1 / sqrt(x*x + y*y), x += xi, y += yi;
		mulps	xmm0, xmm0  ;  to: 1 / sqrt(p), p += v, v += a;
		mulps	xmm1, xmm1
		mulps	xmm2, xmm2
		mulps	xmm3, xmm3
		addps	xmm0, xmm1
		movaps	xmm1, opti4asm[4*16]
		addps	xmm2, xmm3
		subps	xmm2, xmm0

		mov	p1, edx
		mov	ecx, zbufoff
		shl	esi, 2
		add	esi, uurend
		mov	ebx, iplc

		cmp	edi, edx
		jae	short endv

			;Do	first 0-3 pixels to align unrolled loop of 4
		test	edi, 15
		jz	short skip1va

		test	edi, 8
		jz	short skipshufc
		shufps	xmm0, xmm0, 0x4e ;rotate right by 2
skipshufc:
		test	edi, 4
		jz	short skipshufd
		shufps	xmm0, xmm0, 0x39 ;rotate right by 1
skipshufd:

beg1va:
		mov	edx, [esi]
		mov	eax, [esi+MAXXDIM*4]
		add	eax, edx
		sar	edx, 16
		mov	edx, angstart[edx*4]
		mov	[esi], eax

			;Z
		cvtsi2ss	xmm7, [edx+ebx*8+4]
		rsqrtss	xmm3, xmm0
		mulss	xmm7, xmm3
		shufps	xmm0, xmm0, 0x39 ;rotate right by 1
		movss	[edi+ecx], xmm7

			;Col
		punpcklbw	mm0, [edx+ebx*8]
		psrlw	mm0, 8
		movq	mm1, fogcol
		psubw	mm1, mm0
		paddw	mm1, mm1
		mov	eax, [edx+ebx*8+4]
		shr	eax, 16+4
		pmulhw	mm1, foglut[eax*8]
		paddw	mm0, mm1
		packuswb	mm0, mm1
		movd	[edi], mm0

		add	ebx, iinc
		add	esi, 4
		add	edi, 4
		cmp	edi, p1
		jz	short endv
		test	edi, 15
		jnz	short beg1va

		addps	xmm0, xmm2
		addps	xmm2, xmm1
skip1va:
		lea	edx, [edi+16]
		cmp	edx, p1
		ja	short prebeg1v

		cmp	iinc, 0
		jl	short beg4vn

beg4vp:
		movq	mm6, [esi]
		movq	mm7, [esi+8]
		pextrw	eax, mm6, 1
		pextrw	edx, mm6, 3
		paddd	mm6, [esi+MAXXDIM*4]
		mov	eax, angstart[eax*4]
		mov	edx, angstart[edx*4]
		movq	mm4, [eax+ebx*8]
		movq	mm1, [edx+ebx*8+8]
		pextrw	eax, mm7, 1
		pextrw	edx, mm7, 3
		paddd	mm7, [esi+8+MAXXDIM*4]
		mov	eax, angstart[eax*4]
		mov	edx, angstart[edx*4]
		movq	mm5, [eax+ebx*8+16]
		movq	mm3, [edx+ebx*8+24]
		add	ebx, 4

			;Do	Z
		movq	mm0, mm4
		movq	mm2, mm5
		punpckhdq	mm4, mm1
		punpckhdq	mm5, mm3
		cvtpi2ps	xmm7, mm4
		cvtpi2ps	xmm4, mm5
		rsqrtps	xmm3, xmm0
		movlhps	xmm7, xmm4
		mulps	xmm7, xmm3
		movntps	[edi+ecx], xmm7
		addps	xmm0, xmm2
		addps	xmm2, xmm1

		movq	[esi], mm6
		movq	[esi+8], mm7

			;Do	color
		pxor	mm7, mm7
		punpcklbw	mm0, mm7
		punpcklbw	mm1, mm7
		punpcklbw	mm2, mm7
		punpcklbw	mm3, mm7

		movq	mm7, fogcol
		psubw	mm7, mm0
		paddw	mm7, mm7
		pextrw	eax, mm4, 1
		shr	eax, 4
		pmulhw	mm7, foglut[eax*8]
		paddw	mm0, mm7

		movq	mm7, fogcol
		psubw	mm7, mm1
		paddw	mm7, mm7
		pextrw	eax, mm4, 3
		shr	eax, 4
		pmulhw	mm7, foglut[eax*8]
		paddw	mm1, mm7

		movq	mm7, fogcol
		psubw	mm7, mm2
		paddw	mm7, mm7
		pextrw	eax, mm5, 1
		shr	eax, 4
		pmulhw	mm7, foglut[eax*8]
		paddw	mm2, mm7

		movq	mm7, fogcol
		psubw	mm7, mm3
		paddw	mm7, mm7
		pextrw	eax, mm5, 3
		shr	eax, 4
		pmulhw	mm7, foglut[eax*8]
		paddw	mm3, mm7

		packuswb	mm0, mm1
		packuswb	mm2, mm3
		movntq	[edi], mm0
		movntq	[edi+8], mm2

		add	esi, 16
		add	edi, 16
		lea	edx, [edi+16]
		cmp	edx, p1
		jbe	short beg4vp
		cmp	edi, p1
		jae	short endv
		jmp	short prebeg1v

beg4vn:
		movq	mm6, [esi]
		movq	mm7, [esi+8]
		pextrw	eax, mm6, 1
		pextrw	edx, mm6, 3
		paddd	mm6, [esi+MAXXDIM*4]
		mov	eax, angstart[eax*4]
		mov	edx, angstart[edx*4]
		movq	mm4, [eax+ebx*8]
		movq	mm1, [edx+ebx*8-8]
		pextrw	eax, mm7, 1
		pextrw	edx, mm7, 3
		paddd	mm7, [esi+8+MAXXDIM*4]
		mov	eax, angstart[eax*4]
		mov	edx, angstart[edx*4]
		movq	mm5, [eax+ebx*8-16]
		movq	mm3, [edx+ebx*8-24]
		sub	ebx, 4

			;Do	Z
		movq	mm0, mm4
		movq	mm2, mm5
		punpckhdq	mm4, mm1
		punpckhdq	mm5, mm3
		cvtpi2ps	xmm7, mm4
		cvtpi2ps	xmm4, mm5
		rsqrtps	xmm3, xmm0
		movlhps	xmm7, xmm4
		mulps	xmm7, xmm3
		movntps	[edi+ecx], xmm7
		addps	xmm0, xmm2
		addps	xmm2, xmm1

		movq	[esi], mm6
		movq	[esi+8], mm7

			;Do	color
		pxor	mm7, mm7
		punpcklbw	mm0, mm7
		punpcklbw	mm1, mm7
		punpcklbw	mm2, mm7
		punpcklbw	mm3, mm7

		movq	mm7, fogcol
		psubw	mm7, mm0
		paddw	mm7, mm7
		pextrw	eax, mm4, 1
		shr	eax, 4
		pmulhw	mm7, foglut[eax*8]
		paddw	mm0, mm7

		movq	mm7, fogcol
		psubw	mm7, mm1
		paddw	mm7, mm7
		pextrw	eax, mm4, 3
		shr	eax, 4
		pmulhw	mm7, foglut[eax*8]
		paddw	mm1, mm7

		movq	mm7, fogcol
		psubw	mm7, mm2
		paddw	mm7, mm7
		pextrw	eax, mm5, 1
		shr	eax, 4
		pmulhw	mm7, foglut[eax*8]
		paddw	mm2, mm7

		movq	mm7, fogcol
		psubw	mm7, mm3
		paddw	mm7, mm7
		pextrw	eax, mm5, 3
		shr	eax, 4
		pmulhw	mm7, foglut[eax*8]
		paddw	mm3, mm7

		packuswb	mm0, mm1
		packuswb	mm2, mm3
		movntq	[edi], mm0
		movntq	[edi+8], mm2

		add	esi, 16
		add	edi, 16
		lea	edx, [edi+16]
		cmp	edx, p1
		jbe	short beg4vn
		cmp	edi, p1
		jae	short endv

prebeg1v:
beg1v:
		mov	edx, [esi]
		mov	eax, [esi+MAXXDIM*4]
		add	eax, edx
		sar	edx, 16
		mov	edx, angstart[edx*4]
		mov	[esi], eax

			;Z
		cvtsi2ss	xmm7, [edx+ebx*8+4]
		rsqrtss	xmm3, xmm0
		mulss	xmm7, xmm3
		shufps	xmm0, xmm0, 0x39 ;rotate right by 1
		movss	[edi+ecx], xmm7

			;Col
		punpcklbw	mm0, [edx+ebx*8]
		psrlw	mm0, 8
		movq	mm1, fogcol
		psubw	mm1, mm0
		paddw	mm1, mm1
		mov	eax, [edx+ebx*8+4]
		shr	eax, 16+4
		pmulhw	mm1, foglut[eax*8]
		paddw	mm0, mm1
		packuswb	mm0, mm1
		movd	[edi], mm0

		add	ebx, iinc
		add	esi, 4
		add	edi, 4
		cmp	edi, p1
		jne	short beg1v
endv:
		pop	edi
		pop	esi
		pop	ebx
	}
	#endif
	}
	#endif
}