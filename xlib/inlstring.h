
/* Based on functions in linux/string.h */



#if !defined(__386__)||!defined(ASSEMBLY)

#define __memcpy(dst,src,n)			memcpy((dst),(src),(n))
#define __memcpy_conventioanl(dst,src,n)	memcpy((dst),(src),(n))
#define __memcpyb(dst,src,n)			memcpy((dst),(src),(n))
#define __memsetb(dst,c,n)			memset((dst),(c),(n))
#define __memsetlong(dst,c,n)			memset((dst),(c),(n))
#define __memset(dst,c,n)			memset((dst),(c),(n))
#define __memset2(dst,c,n)			memset((dst),(c),2*(n))
#define __memset3(dst,c,n)			memset((dst),(c),3*(n))

#else

#include <linux/types.h>	/* for size_t */
static INLINE void *
__memcpy_conventional (void *to, const void *from, size_t n)
{
  __asm__ ("cld\n\t"
	   "movl %%edi,%%ecx\n\t"
	   "andl $1,%%ecx\n\t"
	   "subl %%ecx,%%edx\n\t"
	   "rep ; movsb\n\t"	/* 16-bit align destination */
	   "movl %%edx,%%ecx\n\t"
	   "shrl $2,%%ecx\n\t"
	   "rep ; movsl\n\t"
	   "testb $1,%%dl\n\t"
	   "je 1f\n\t"
	   "movsb\n"
	   "1:\ttestb $2,%%dl\n\t"
	   "je 2f\n\t"
	   "movsw\n"
	   "2:\n"
: :	   "d" (n), "D" ((long) to), "S" ((long) from)
:	   "cx", "dx", "di", "si");
  return (to);
}


static INLINE void *
__memcpyb (void *to, const void *from, size_t n)
{
  __asm__ ("cld\n\t"
	   "rep ; movsb\n\t"
: :	   "c" (n), "D" ((long) to), "S" ((long) from)
:	   "cx", "di", "si");
  return (to);
}

static INLINE void *
__memsetb (void *s, char c, size_t count)
{
  __asm__ ("cld\n\t"
	   "rep\n\t"
	   "stosb"
: :	   "a" (c), "D" (s), "c" (count)
:	   "cx", "di");
  return s;
}

static INLINE void *
__memsetlong (void *s, unsigned c, size_t count)
{
  __asm__ ("cld\n\t"
	   "rep\n\t"
	   "stosl"
: :	   "a" (c), "D" (s), "c" (count)
:	   "cx", "di");
  return s;
}

static INLINE void *
__memset (void *s, char c, size_t count)
{
  __asm__ (
	    "cld\n\t"
	    "cmpl $12,%%edx\n\t"
	    "jl 1f\n\t"		/* if (count >= 12) */

	    "movzbl %%al,%%ax\n\t"
	    "movl %%eax,%%ecx\n\t"
	    "shll $8,%%ecx\n\t"	/* c |= c << 8 */
	    "orl %%ecx,%%eax\n\t"
	    "movl %%eax,%%ecx\n\t"
	    "shll $16,%%ecx\n\t"	/* c |= c << 16 */
	    "orl %%ecx,%%eax\n\t"

	    "movl %%edx,%%ecx\n\t"
	    "negl %%ecx\n\t"
	    "andl $3,%%ecx\n\t"	/* (-s % 4) */
	    "subl %%ecx,%%edx\n\t"	/* count -= (-s % 4) */
	    "rep ; stosb\n\t"	/* align to longword boundary */

	    "movl %%edx,%%ecx\n\t"
	    "shrl $2,%%ecx\n\t"
	    "rep ; stosl\n\t"	/* fill longwords */

	    "andl $3,%%edx\n"	/* fill last few bytes */
	    "1:\tmovl %%edx,%%ecx\n\t"	/* <= 12 entry point */
	    "rep ; stosb\n\t"
: :	    "a" (c), "D" (s), "d" (count)
:	    "ax", "cx", "dx", "di");
  return s;
}

static INLINE void *
__memset2 (void *s, short c, size_t count)
/* count is in 16-bit pixels */
/* s is assumed to be 16-bit aligned */
{
  __asm__ (
	    "cld\n\t"
	    "cmpl $12,%%edx\n\t"
	    "jl 1f\n\t"		/* if (count >= 12) */

	    "movzwl %%ax,%%eax\n\t"
	    "movl %%eax,%%ecx\n\t"
	    "shll $16,%%ecx\n\t"	/* c |= c << 16 */
	    "orl %%ecx,%%eax\n\t"

	    "movl %%edi,%%ecx\n\t"
	    "andl $2,%%ecx\n\t"	/* s & 2 */
	    "jz 2f\n\t"
	    "decl %%edx\n\t"	/* count -= 1 */
	    "stosw\n\t"		/* align to longword boundary */

	    "2:\n\t"
	    "movl %%edx,%%ecx\n\t"
	    "shrl $1,%%ecx\n\t"
	    "rep ; stosl\n\t"	/* fill longwords */

	    "andl $1,%%edx\n"	/* one 16-bit word left? */
	    "jz 3f\n\t"		/* no, finished */
	    "1:\tmovl %%edx,%%ecx\n\t"	/* <= 12 entry point */
	    "rep ; stosw\n\t"
	    "3:\n\t"
: :	    "a" (c), "D" (s), "d" (count)
:	    "ax", "cx", "dx", "di");
  return s;
}

static INLINE void *
__memset3 (void *s, int c, size_t count)
/* count is in 24-bit pixels (3 bytes per pixel) */
{
  __asm__ (
	    "cmpl $8,%%edx\n\t"
  /*      "jmp 2f\n\t" *//* debug */
	    "jl 2f\n\t"

	    "movl %%eax,%%ebx\n\t"	/* eax = (low) BGR0 (high) */
	    "shll $24,%%ebx\n\t"	/* ebx = 000B */
	    "orl %%ebx,%%eax\n\t"	/* eax = BGRB */

	    "movl %%eax,%%ebx\n\t"
	    "shrl $8,%%ebx\n\t"	/* ebx = GRB0 */
	    "movl %%ebx,%%ecx\n\t"
	    "shll $24,%%ecx\n\t"	/* ecx = 000G */
	    "orl %%ecx,%%ebx\n\t"	/* ebx = GRBG */

	    "movl %%eax,%%ecx\n\t"
	    "shll $8,%%ecx\n\t"	/* ecx = 0BGR */
	    "movb %%bh,%%cl\n\t"	/* ecx = RBGR */

	    "cmpl $16,%%edx\n\t"
	    "jl 1f\n\t"
	    "jmp 5f\n\t"
	    ".align 4,0x90\n\t"

	    "5:\n\t"		/* loop unrolling */
	    "movl %%eax,(%%edi)\n\t"	/* write BGRB */
	    "movl %%ebx,4(%%edi)\n\t"	/* write GRBG */
	    "movl %%ecx,8(%%edi)\n\t"	/* write RBGR */
	    "movl %%eax,12(%%edi)\n\t"
	    "movl %%ebx,16(%%edi)\n\t"
	    "movl %%ecx,20(%%edi)\n\t"
	    "movl %%eax,24(%%edi)\n\t"
	    "movl %%ebx,28(%%edi)\n\t"
	    "movl %%ecx,32(%%edi)\n\t"
	    "movl %%eax,36(%%edi)\n\t"
	    "subl $16,%%edx\n\t"	/* blend end-of-loop instr. */
	    "movl %%ebx,40(%%edi)\n\t"
	    "movl %%ecx,44(%%edi)\n\t"
	    "addl $48,%%edi\n\t"
	    "cmpl $16,%%edx\n\t"
	    "jge 5b\n\t"
	    "andl %%edx,%%edx\n\t"
	    "jz 4f\n\t"		/* finished */
	    "cmpl $4,%%edx\n\t"
	    "jl 2f\n\t"		/* less than 4 pixels left */
	    "jmp 1f\n\t"
	    ".align 4,0x90\n\t"

	    "1:\n\t"
	    "movl %%eax,(%%edi)\n\t"	/* write BGRB */
	    "movl %%ebx,4(%%edi)\n\t"	/* write GRBG */
	    "movl %%ecx,8(%%edi)\n\t"	/* write RBGR */
	    "addl $12,%%edi\n\t"
	    "subl $4,%%edx\n\t"
	    "cmpl $4,%%edx\n\t"
	    "jge 1b\n\t"

	    "2:\n\t"
	    "cmpl $0,%%edx\n\t"	/* none left? */
	    "jle 4f\n\t"	/* finished */

	    "mov %%eax,%%ecx\n\t"
	    "shrl $16,%%ecx\n\t"	/* B in cl */

	    "3:\n\t"		/* write last few pixels */
	    "movw %%ax,(%%edi)\n\t"	/* write RG */
	    "movb %%cl,2(%%edi)\n\t"	/* write B */
	    "addl $3,%%edi\n\t"
	    "decl %%edx\n\t"
	    "jnz 3b\n\t"

	    "4:\n\t"
: :	    "a" (c), "D" (s), "d" (count)
:	    "ax", "bx", "cx", "dx", "di");
  return s;
}

/* Functions defined in mem.S */

extern          memcpy4to3 (void *dest, void *src, int n);
extern          memcpy32shift8 (void *dest, void *src, int n);

/* Functions for which arguments must be passed in %ebx, %edx, and %ecx. */
extern          __memcpyasm_regargs ();		/* nu_bytes >= 3 */
extern          __memcpyasm_regargs_aligned ();		/* nu_bytes >= 32 */


/* Always 32-bit align destination, even for a small number of bytes. */
static INLINE void *
__memcpy_aligndest (void *dest, const void *src, int n)
{
  __asm__         __volatile__ (
				"cmpl $3, %%ecx\n\t"
				"ja 1f\n\t"
				"call * __memcpy_jumptable (, %%ecx, 4)\n\t"
				"jmp 2f\n\t"
				"1:call __memcpyasm_regargs\n\t"
				:
				:"b"            (dest), "d" (src), "c" (n)
				:"ax", "0", "1", "2");
}


/* Optimized version for 32-bit aligned destination. */
static INLINE void *
__memcpy_destaligned (void *dest, const void *src, int n)
{
  __asm__         __volatile__ (
				"cmpl $32, %%ecx\n\t"
				"ja 1f\n\t"
				"call * __memcpy_jumptable (, %%ecx, 4)\n\t"
				"jmp 2f\n\t"
				"1:call __memcpyasm_regargs_aligned\n\t"
				"2:\n\t"
				:
				:"b"            (dest), "d" (src), "c" (n)
				:"ax", "0", "1", "2");
}


/* Balanced INLINE memcpy; 32-bit align destination if nu_bytes >= 20. */
static INLINE void *
__memcpy_balanced (void *dest, const void *src, int n)
{
  __asm__         __volatile__ (
				"cmpl $19, %%ecx\n\t"
				"ja 1f\n\t"
				"call * __memcpy_jumptable (, %%ecx, 4)\n\t"
				"jmp 2f\n\t"
				"1:call __memcpyasm_regargs\n\t"
				"2:\n\t"
				:
	     :"b"            ((long) dest), "d" ((long) src), "c" ((long) n)
				:"ax", "bx", "cx", "dx");
}


#define __memcpy __memcpy_conventional

#endif
