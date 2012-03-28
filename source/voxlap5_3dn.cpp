// 3DNow! Specific Code
namespace cpu_3dn_specific_code
{
	#ifndef _DOS
	void hrendz (long sx, long sy, long p1, long plc, long incr, long j)
	{
		#if defined(__GNUC__) && !defined(__NOASM__) //AT&T SYNTAX ASSEMBLY
		__asm__ __volatile__
		(
			"push	%esi\n\t"
			"push	%edi\n\t"
			"mov	sy,%eax\n\t"
			"mov	ylookup(,%eax,4),%eax\n\t"
			"add	frameplace,%eax\n\t"
			"mov	p1,%esi\n\t"
			"lea	(%eax,%esi,4),%esi   \n\t" //esi = p1
			"mov	sx,%edi\n\t"
			"lea	(%eax,%edi,4),%edi   \n\t" //edi = p0
	
			"movl	sx,%mm0\n\t"
			"punpckldq	sy,%mm0\n\t"
			"pi2fd	%mm0,%mm0         \n\t" //mm0: (float)sy (float)sx
			"pshufw	%mm0,0xee,%mm2  \n\t" //mm2: (float)sy (float)sy
			"punpckldq	%mm0,%mm0     \n\t" //mm0: (float)sx (float)sx
			"movl	optistrx,%mm1\n\t"
			"punpckldq	optistry,%mm1\n\t"
			"pfmul	%mm1,%mm0         \n\t" //mm0: (float)sx*optistry (float)sx*optistrx
			"movl	optiheix,%mm3\n\t"
			"punpckldq	optiheiy,%mm3\n\t"
			"pfmul	%mm3,%mm2         \n\t" //mm2: (float)sy*optiheiy (float)sy*optiheix
			"pfadd	%mm2,%mm0\n\t"
			"movl	optiaddx,%mm3\n\t"
			"punpckldq	optiaddy,%mm3\n\t" //mm3: optiaddy optiaddx
			"pfadd	%mm3,%mm0         \n\t" //mm0: diry diry
	
			"movl	plc,%mm6\n\t"
			"movl	incr,%mm7\n\t"
			"mov	zbufoff,%ecx\n\t"
			"mov	j,%edx\n"
	
	"beg:\n\t"
			"pextrw	%mm6,1,%eax\n\t"
			"mov	angstart(,%eax,4),%eax\n\t"
			"movq	(%eax,%edx,8),%mm2  \n\t" //mm2:      dist       col
			"pshufw	%mm2,0xee,%mm3  \n\t" //mm3:         ?      dist
			"pi2fd	%mm3,%mm3         \n\t" //mm3:         ?   (f)dist
			"movq	%mm0,%mm4          \n\t" //mm4:      diry      dirx
			"pfmul	%mm4,%mm4         \n\t" //mm4:    diry^2    dirx^2
			"pfadd	%mm1,%mm0         \n\t" //mm0: dirx+optx diry+opty (unrelated)
			"pfacc	%mm4,%mm4         \n\t" //mm4: (x^2+y^2)   x^2+y^2
			"pfrsqrt	%mm4,%mm4       \n\t" //mm4: 1/sqrt(*) 1/sqrt(*)
			"pfmul	%mm4,%mm3         \n\t" //mm3:         0    zvalue
			"paddd	%mm7,%mm6         \n\t" //mm6:            plc+incr (unrelated)
			"movl	%mm2,(%edi)\n\t"
			"movl	%mm3,(%edi,%ecx)\n\t"
			"add	$4,%edi\n\t"
			"cmp	%esi,%edi\n\t"
			"jb	beg\n\t"
			"pop	%edi\n\t"
			"pop	%esi\n\t"
		);
		#endif
		#if defined(_MSC_VER) && !defined(__NOASM__) //MASM SYNTAX ASSEMBLY
		_asm
		{
			push	esi
			push	edi
			mov	eax, sy
			mov	eax, ylookup[eax*4]
			add	eax, frameplace
			mov	esi, p1
			lea	esi, [eax+esi*4]      //esi = p1
			mov	edi, sx
			lea	edi, [eax+edi*4]      //edi = p0
	
			movd	mm0, sx
			punpckldq	mm0, sy
			pi2fd	mm0, mm0          //mm0: (float)sy (float)sx
			pshufw	mm2, mm0, 0xee    //mm2: (float)sy (float)sy
			punpckldq	mm0, mm0      //mm0: (float)sx (float)sx
			movd	mm1, optistrx
			punpckldq	mm1, optistry
			pfmul	mm0, mm1          //mm0: (float)sx*optistry (float)sx*optistrx
			movd	mm3, optiheix
			punpckldq	mm3, optiheiy
			pfmul	mm2, mm3          //mm2: (float)sy*optiheiy (float)sy*optiheix
			pfadd	mm0, mm2
			movd	mm3, optiaddx
			punpckldq	mm3, optiaddy //mm3: optiaddy optiaddx
			pfadd	mm0, mm3          //mm0: diry diry
	
			movd	mm6, plc
			movd	mm7, incr
			mov	ecx, zbufoff
			mov	edx, j
	
	beg:
			pextrw	eax, mm6, 1
			mov	eax, angstart[eax*4]
			movq	mm2, [eax+edx*8] //mm2:      dist       col
			pshufw	mm3, mm2, 0xee   //mm3:         ?      dist
			pi2fd	mm3, mm3         //mm3:         ?   (f)dist
			movq	mm4, mm0         //mm4:      diry      dirx
			pfmul	mm4, mm4         //mm4:    diry^2    dirx^2
			pfadd	mm0, mm1         //mm0: dirx+optx diry+opty (unrelated)
			pfacc	mm4, mm4         //mm4: (x^2+y^2)   x^2+y^2
			pfrsqrt	mm4, mm4         //mm4: 1/sqrt(*) 1/sqrt(*)
			pfmul	mm3, mm4         //mm3:         0    zvalue
			paddd	mm6, mm7         //mm6:            plc+incr (unrelated)
			movd	[edi], mm2
			movd	[edi+ecx], mm3
			add	edi, 4
			cmp	edi, esi
			jb	short beg
			pop	edi
			pop	esi
		}
		#endif
	}
	
	void hrendzfog (long sx, long sy, long p1, long plc, long incr, long j)
	{
		#if defined(__GNUC__) && !defined(__NOASM__) //AT&T SYNTAX ASSEMBLY
		__asm__ __volatile__
		(
			"push	%esi\n\t"
			"push	%edi\n\t"
			"mov	sy,%eax\n\t"
			"mov	ylookup(,%eax,4),%eax\n\t"
			"add	frameplace,%eax\n\t"
			"mov	p1,%esi\n\t"
			"lea	(%eax,%esi,4),%esi   \n\t" //esi = p1
			"mov	sx,%edi\n\t"
			"lea	(%eax,%edi,4),%edi   \n\t" //edi = p0
	
			"movl	sx,%mm0\n\t"
			"punpckldq	sy,%mm0\n\t"
			"pi2fd	%mm0,%mm0         \n\t" //mm0: (float)sy (float)sx
			"pshufw	%mm0,0xee,%mm2  \n\t" //mm2: (float)sy (float)sy
			"punpckldq	%mm0,%mm0     \n\t" //mm0: (float)sx (float)sx
			"movl	optistrx,%mm1\n\t"
			"punpckldq	optistry,%mm1\n\t"
			"pfmul	%mm1,%mm0         \n\t" //mm0: (float)sx*optistry (float)sx*optistrx
			"movl	optiheix,%mm3\n\t"
			"punpckldq	optiheiy,%mm3\n\t"
			"pfmul	%mm3,%mm2         \n\t" //mm2: (float)sy*optiheiy (float)sy*optiheix
			"pfadd	%mm2,%mm0\n\t"
			"movl	optiaddx,%mm3\n\t"
			"punpckldq	optiaddy,%mm3\n\t" //mm3: optiaddy optiaddx
			"pfadd	%mm3,%mm0         \n\t" //mm0: diry diry
	
			"pxor	%mm5,%mm5\n\t"
	
			"movl	plc,%mm6\n\t"
			"movl	incr,%mm7\n\t"
			"mov	zbufoff,%ecx\n\t"
			"mov	j,%edx\n"
	
	"beg:\n\t"
			"pextrw	%mm6,1,%eax\n\t"
			"mov	angstart(,%eax,4),%eax\n\t"
			"movq	(%eax,%edx,8),%mm2  \n\t" //mm2:      dist       col
			"pshufw	%mm2,0xee,%mm3  \n\t" //mm3:         ?      dist
			"pi2fd	%mm3,%mm3         \n\t" //mm3:         ?   (f)dist
			"movq	%mm0,%mm4          \n\t" //mm4:      diry      dirx
			"pfmul	%mm4,%mm4         \n\t" //mm4:    diry^2    dirx^2
			"pfadd	%mm1,%mm0         \n\t" //mm0: dirx+optx diry+opty (unrelated)
			"pfacc	%mm4,%mm4         \n\t" //mm4: (x^2+y^2)   x^2+y^2
			"pfrsqrt	%mm4,%mm4       \n\t" //mm4: 1/sqrt(*) 1/sqrt(*)
			"pfmul	%mm4,%mm3         \n\t" //mm3:         0    zvalue
			"paddd	%mm7,%mm6         \n\t" //mm6:            plc+incr (unrelated)
	
					//Extra calculations for fog\n\t"
			"pextrw	%mm2,3,%eax\n\t"
			"punpcklbw	%mm5,%mm2\n\t"
			"movq	fogcol,%mm4\n\t"
			"psubw	%mm2,%mm4\n\t"
			"paddw	%mm4,%mm4\n\t"
			"shr	$4,%eax\n\t"
			"pmulhw	foglut(,%eax,8),%mm4\n\t"
			"paddw	%mm4,%mm2\n\t"
			"packuswb	%mm4,%mm2\n\t"
	
			"movl	%mm2,(%edi)\n\t"
			"movl	%mm3,(%edi,%ecx)\n\t"
			"add	$4,%edi\n\t"
			"cmp	%esi,%edi\n\t"
			"jb	beg\n\t"
			"pop	%edi\n\t"
			"pop	%esi\n\t"
		);
		#endif
		#if defined(_MSC_VER) && !defined(__NOASM__) //MASM SYNTAX ASSEMBLY
		_asm
		{
			push	esi
			push	edi
			mov	eax, sy
			mov	eax, ylookup[eax*4]
			add	eax, frameplace
			mov	esi, p1
			lea	esi, [eax+esi*4]    ;esi = p1
			mov	edi, sx
			lea	edi, [eax+edi*4]    ;edi = p0
	
			movd	mm0, sx
			punpckldq	mm0, sy
			pi2fd	mm0, mm0          ;mm0: (float)sy (float)sx
			pshufw	mm2, mm0, 0xee   ;mm2: (float)sy (float)sy
			punpckldq	mm0, mm0      ;mm0: (float)sx (float)sx
			movd	mm1, optistrx
			punpckldq	mm1, optistry
			pfmul	mm0, mm1          ;mm0: (float)sx*optistry (float)sx*optistrx
			movd	mm3, optiheix
			punpckldq	mm3, optiheiy
			pfmul	mm2, mm3          ;mm2: (float)sy*optiheiy (float)sy*optiheix
			pfadd	mm0, mm2
			movd	mm3, optiaddx
			punpckldq	mm3, optiaddy ;mm3: optiaddy optiaddx
			pfadd	mm0, mm3          ;mm0: diry diry
	
			pxor	mm5, mm5
	
			movd	mm6, plc
			movd	mm7, incr
			mov	ecx, zbufoff
			mov	edx, j
	
	beg:
			pextrw	eax, mm6, 1
			mov	eax, angstart[eax*4]
			movq	mm2, [eax+edx*8]   ;mm2:      dist       col
			pshufw	mm3, mm2, 0xee   ;mm3:         ?      dist
			pi2fd	mm3, mm3          ;mm3:         ?   (f)dist
			movq	mm4, mm0           ;mm4:      diry      dirx
			pfmul	mm4, mm4          ;mm4:    diry^2    dirx^2
			pfadd	mm0, mm1          ;mm0: dirx+optx diry+opty (unrelated)
			pfacc	mm4, mm4          ;mm4: (x^2+y^2)   x^2+y^2
			pfrsqrt	mm4, mm4        ;mm4: 1/sqrt(*) 1/sqrt(*)
			pfmul	mm3, mm4          ;mm3:         0    zvalue
			paddd	mm6, mm7          ;mm6:            plc+incr (unrelated)
	
				;Extra	calculations for fog
			pextrw	eax, mm2, 3
			punpcklbw	mm2, mm5
			movq	mm4, fogcol
			psubw	mm4, mm2
			paddw	mm4, mm4
			shr	eax, 4
			pmulhw	mm4, foglut[eax*8]
			paddw	mm2, mm4
			packuswb	mm2, mm4
	
			movd	[edi], mm2
			movd	[edi+ecx], mm3
			add	edi, 4
			cmp	edi, esi
			jb	short beg
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
			"push	%edi\n\t"
			"mov	p1,%esi\n\t"
			"mov	sx,%edi\n\t"
			"cmp	%esi,%edi\n\t"
			"jae	endv\n\t"
			"mov	sy,%eax\n\t"
			"mov	ylookup(,%eax,4),%eax\n\t"
			"add	frameplace,%eax\n\t"
			"lea	(%eax,%esi,4),%esi   \n\t" //esi = p1
			"lea	(%eax,%edi,4),%edi   \n\t" //edi = p0
	
			"movl	sx,%mm0\n\t"
			"punpckldq	sy,%mm0\n\t"
			"pi2fd	%mm0,%mm0         \n\t" //mm0: (float)sy (float)sx
			"pshufw	%mm0,0xee,%mm2  \n\t" //mm2: (float)sy (float)sy
			"punpckldq	%mm0,%mm0     \n\t" //mm0: (float)sx (float)sx
			"movl	optistrx,%mm1\n\t"
			"punpckldq	optistry,%mm1\n\t"
			"pfmul	%mm1,%mm0         \n\t" //mm0: (float)sx*optistry (float)sx*optistrx
			"movl	optiheix,%mm3\n\t"
			"punpckldq	optiheiy,%mm3\n\t"
			"pfmul	%mm3,%mm2         \n\t" //mm2: (float)sy*optiheiy (float)sy*optiheix
			"pfadd	%mm2,%mm0\n\t"
			"movl	optiaddx,%mm3\n\t"
			"punpckldq	optiaddy,%mm3\n\t" //mm3: optiaddy optiaddx
			"pfadd	%mm3,%mm0         \n\t" //mm0: diry diry
	
			"mov	zbufoff,%ecx\n\t"
			"mov	iplc,%edx\n\t"
			"mov	sx,%ebx\n\t"
			"mov	uurend,%eax\n\t"
			"lea	(%eax,%ebx,4),%ebx\n"
	
	"begv_3dn:\n\t"
			"movl	(%ebx),%mm5\n\t"
			"pextrw	%mm5,1,%eax\n\t"
			"paddd	MAXXDIM(,%ebx,4),%mm5\n\t"
			"movl	%mm5,(%ebx)\n\t"
			"mov	angstart(,%eax,4),%eax\n\t"
			"movq	(%eax,%edx,8),%mm2  \n\t" //mm2:      dist       col
			"pshufw	%mm2,0xee,%mm3  \n\t" //mm3:         ?      dist
			"pi2fd	%mm3,%mm3         \n\t" //mm3:         ?   (f)dist
			"movq	%mm0,%mm4          \n\t" //mm4:      diry      dirx
			"pfmul	%mm4,%mm4         \n\t" //mm4:    diry^2    dirx^2
			"pfadd	%mm1,%mm0         \n\t" //mm0: dirx+optx diry+opty (unrelated)
			"pfacc	%mm4,%mm4         \n\t" //mm4: (x^2+y^2)   x^2+y^2
			"pfrsqrt	%mm4,%mm4       \n\t" //mm4: 1/sqrt(*) 1/sqrt(*)
			"pfmul	%mm4,%mm3         \n\t" //mm3:         0    zvalue
			"movl	%mm2,(%edi)\n\t"
			"movl	%mm3,(%edi,%ecx)\n\t"
			"add	iinc,%edx\n\t"
			"add	$4,%ebx\n\t"
			"add	$4,%edi\n\t"
			"cmp	%esi,%edi\n\t"
			"jb	begv_3dn\n"
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
			mov	esi, p1
			mov	edi, sx
			cmp	edi, esi
			jae	short endv
			mov	eax, sy
			mov	eax, ylookup[eax*4]
			add	eax, frameplace
			lea	esi, [eax+esi*4]    ;esi = p1
			lea	edi, [eax+edi*4]    ;edi = p0
	
			movd	mm0, sx
			punpckldq	mm0, sy
			pi2fd	mm0, mm0          ;mm0: (float)sy (float)sx
			pshufw	mm2, mm0, 0xee   ;mm2: (float)sy (float)sy
			punpckldq	mm0, mm0      ;mm0: (float)sx (float)sx
			movd	mm1, optistrx
			punpckldq	mm1, optistry
			pfmul	mm0, mm1          ;mm0: (float)sx*optistry (float)sx*optistrx
			movd	mm3, optiheix
			punpckldq	mm3, optiheiy
			pfmul	mm2, mm3          ;mm2: (float)sy*optiheiy (float)sy*optiheix
			pfadd	mm0, mm2
			movd	mm3, optiaddx
			punpckldq	mm3, optiaddy ;mm3: optiaddy optiaddx
			pfadd	mm0, mm3          ;mm0: diry diry
	
			mov	ecx, zbufoff
			mov	edx, iplc
			mov	ebx, sx
			mov	eax, uurend
			lea	ebx, [eax+ebx*4]
	
	begv_3dn:
			movd	mm5, [ebx]
			pextrw	eax, mm5, 1
			paddd	mm5, [ebx+MAXXDIM*4]
			movd	[ebx], mm5
			mov	eax, angstart[eax*4]
			movq	mm2, [eax+edx*8]   ;mm2:      dist       col
			pshufw	mm3, mm2, 0xee   ;mm3:         ?      dist
			pi2fd	mm3, mm3          ;mm3:         ?   (f)dist
			movq	mm4, mm0           ;mm4:      diry      dirx
			pfmul	mm4, mm4          ;mm4:    diry^2    dirx^2
			pfadd	mm0, mm1          ;mm0: dirx+optx diry+opty (unrelated)
			pfacc	mm4, mm4          ;mm4: (x^2+y^2)   x^2+y^2
			pfrsqrt	mm4, mm4        ;mm4: 1/sqrt(*) 1/sqrt(*)
			pfmul	mm3, mm4          ;mm3:         0    zvalue
			movd	[edi], mm2
			movd	[edi+ecx], mm3
			add	edx, iinc
			add	ebx, 4
			add	edi, 4
			cmp	edi, esi
			jb	short begv_3dn
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
		"push	%edi\n\t"
		"mov	p1,%esi\n\t"
		"mov	sx,%edi\n\t"
		"cmp	%esi,%edi\n\t"
		"jae	endv\n\t"
		"mov	sy,%eax\n\t"
		"mov	ylookup(,%eax,4),%eax\n\t"
		"add	frameplace,%eax\n\t"
		"lea	(%eax,%esi,4),%esi   \n\t" //esi = p1
		"lea	(%eax,%edi,4),%edi   \n\t" //edi = p0

		"movl	sx,%mm0\n\t"
		"punpckldq	sy,%mm0\n\t"
		"pi2fd	%mm0,%mm0         \n\t" //mm0: (float)sy (float)sx
		"pshufw	%mm0,0xee,%mm2  \n\t" //mm2: (float)sy (float)sy
		"punpckldq	%mm0,%mm0     \n\t" //mm0: (float)sx (float)sx
		"movl	optistrx,%mm1\n\t"
		"punpckldq	optistry,%mm1\n\t"
		"pfmul	%mm1,%mm0         \n\t" //mm0: (float)sx*optistry (float)sx*optistrx
		"movl	optiheix,%mm3\n\t"
		"punpckldq	optiheiy,%mm3\n\t"
		"pfmul	%mm3,%mm2         \n\t" //mm2: (float)sy*optiheiy (float)sy*optiheix
		"pfadd	%mm2,%mm0\n\t"
		"movl	optiaddx,%mm3\n\t"
		"punpckldq	optiaddy,%mm3\n\t" //mm3: optiaddy optiaddx
		"pfadd	%mm3,%mm0         \n\t" //mm0: diry diry

		"pxor	%mm6,%mm6\n\t"

		"mov	zbufoff,%ecx\n\t"
		"mov	iplc,%edx\n\t"
		"mov	sx,%ebx\n\t"
		"mov	uurend,%eax\n\t"
		"lea	(%eax,%ebx,4),%ebx\n"

"begv_3dn:\n\t"
		"movl	(%ebx),%mm5\n\t"
		"pextrw	%mm5,1,%eax\n\t"
		"paddd	MAXXDIM(,%ebx,4),%mm5\n\t"
		"movl	%mm5,(%ebx)\n\t"
		"mov	angstart(,%eax,4),%eax\n\t"
		"movq	(%eax,%edx,8),%mm2  \n\t" //mm2:      dist       col
		"pshufw	%mm2,0xee,%mm3  \n\t" //mm3:         ?      dist
		"pi2fd	%mm3,%mm3         \n\t" //mm3:         ?   (f)dist
		"movq	%mm0,%mm4          \n\t" //mm4:      diry      dirx
		"pfmul	%mm4,%mm4         \n\t" //mm4:    diry^2    dirx^2
		"pfadd	%mm1,%mm0         \n\t" //mm0: dirx+optx diry+opty (unrelated)
		"pfacc	%mm4,%mm4         \n\t" //mm4: (x^2+y^2)   x^2+y^2
		"pfrsqrt	%mm4,%mm4       \n\t" //mm4: 1/sqrt(*) 1/sqrt(*)
		"pfmul	%mm4,%mm3         \n\t" //mm3:         0    zvalue

				//Extra calculations for fog\n\t"
		"pextrw	%mm2,3,%eax\n\t"
		"punpcklbw	%mm6,%mm2\n\t"
		"movq	fogcol,%mm4\n\t"
		"psubw	%mm2,%mm4\n\t"
		"paddw	%mm4,%mm4\n\t"
		"shr	$4,%eax\n\t"
		"pmulhw	foglut(,%eax,8),%mm4\n\t"
		"paddw	%mm4,%mm2\n\t"
		"packuswb	%mm4,%mm2\n\t"

		"movl	%mm2,(%edi)\n\t"
		"movl	%mm3,(%edi,%ecx)\n\t"
		"add	iinc,%edx\n\t"
		"add	$4,%ebx\n\t"
		"add	$4,%edi\n\t"
		"cmp	%esi,%edi\n\t"
		"jb	begv_3dn\n"
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
		mov	esi, p1
		mov	edi, sx
		cmp	edi, esi
		jae	short endv
		mov	eax, sy
		mov	eax, ylookup[eax*4]
		add	eax, frameplace
		lea	esi, [eax+esi*4]    ;esi = p1
		lea	edi, [eax+edi*4]    ;edi = p0

		movd	mm0, sx
		punpckldq	mm0, sy
		pi2fd	mm0, mm0          ;mm0: (float)sy (float)sx
		pshufw	mm2, mm0, 0xee   ;mm2: (float)sy (float)sy
		punpckldq	mm0, mm0      ;mm0: (float)sx (float)sx
		movd	mm1, optistrx
		punpckldq	mm1, optistry
		pfmul	mm0, mm1          ;mm0: (float)sx*optistry (float)sx*optistrx
		movd	mm3, optiheix
		punpckldq	mm3, optiheiy
		pfmul	mm2, mm3          ;mm2: (float)sy*optiheiy (float)sy*optiheix
		pfadd	mm0, mm2
		movd	mm3, optiaddx
		punpckldq	mm3, optiaddy ;mm3: optiaddy optiaddx
		pfadd	mm0, mm3          ;mm0: diry diry

		pxor	mm6, mm6

		mov	ecx, zbufoff
		mov	edx, iplc
		mov	ebx, sx
		mov	eax, uurend
		lea	ebx, [eax+ebx*4]

begv_3dn:
		movd	mm5, [ebx]
		pextrw	eax, mm5, 1
		paddd	mm5, [ebx+MAXXDIM*4]
		movd	[ebx], mm5
		mov	eax, angstart[eax*4]
		movq	mm2, [eax+edx*8]   ;mm2:      dist       col
		pshufw	mm3, mm2, 0xee   ;mm3:         ?      dist
		pi2fd	mm3, mm3          ;mm3:         ?   (f)dist
		movq	mm4, mm0           ;mm4:      diry      dirx
		pfmul	mm4, mm4          ;mm4:    diry^2    dirx^2
		pfadd	mm0, mm1          ;mm0: dirx+optx diry+opty (unrelated)
		pfacc	mm4, mm4          ;mm4: (x^2+y^2)   x^2+y^2
		pfrsqrt	mm4, mm4        ;mm4: 1/sqrt(*) 1/sqrt(*)
		pfmul	mm3, mm4          ;mm3:         0    zvalue

			;Extra	calculations for fog
		pextrw	eax, mm2, 3
		punpcklbw	mm2, mm6
		movq	mm4, fogcol
		psubw	mm4, mm2
		paddw	mm4, mm4
		shr	eax, 4
		pmulhw	mm4, foglut[eax*8]
		paddw	mm2, mm4
		packuswb	mm2, mm4

		movd	[edi], mm2
		movd	[edi+ecx], mm3
		add	edx, iinc
		add	ebx, 4
		add	edi, 4
		cmp	edi, esi
		jb	short begv_3dn
endv:
		pop	edi
		pop	esi
		pop	ebx
	}
	#endif
	}
	#endif
}