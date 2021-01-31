#ifndef _AXPIKE_IFACE_H_
#define _AXPIKE_IFACE_H_

#ifndef AXPIKE_SYSNUM
#define AXPIKE_SYSNUM 380
#endif

#define AXPIKE_ILM_AA 0x00
#define AXPIKE_ILM_EA 0x01
#define AXPIKE_AXRAM 0x02
#define AXPIKE_KULKARNI 0x03
#define AXPIKE_MUL8_303 0x04
#define AXPIKE_MUL8_469 0x05
#define AXPIKE_MUL8_479 0x06
#define AXPIKE_MUL8_423 0x07
#define AXPIKE_MUL8_279 0x08


#ifndef _AXPIKE_SUPPRESS_
#define axpike_activate(x) asm volatile ("li a7,%0\nli a0,%1\necall" : : "i"(AXPIKE_SYSNUM), "i"((x&0x3f)|0x40) : "a0","a7")
#define axpike_bactivate(x) asm volatile ("li a7,%0\nli a0,%1\necall" : : "i"(AXPIKE_SYSNUM), "i"((x&0x3f)|0x140) : "a0","a7")
#define axpike_deactivate(x) asm volatile ("li a7,%0\nli a0,%1\necall" : : "i"(AXPIKE_SYSNUM), "i"(x&0x3f) : "a0","a7")
#define axpike_bdeactivate(x) asm volatile ("li a7,%0\nli a0,%1\necall" : : "i"(AXPIKE_SYSNUM), "i"((x&0x3f)|0x100) : "a0","a7")
#define axpike_clear() asm volatile ("li a7,%0\nli a0,0xff\necall" : : "i"(AXPIKE_SYSNUM) : "a0","a7")
#define axpike_bclear() asm volatile ("li a7,%0\nli a0,0x1ff\necall" : : "i"(AXPIKE_SYSNUM) : "a0","a7")

#else
#define axpike_activate(x)
#define axpike_bactivate(x)
#define axpike_deactivate(x)
#define axpike_bdeactivate(x)
#define axpike_clear()
#define axpike_bclear()
#endif

#define axpike_newsection() asm volatile ("li a7,%0\nli a0,0x1fe\necall" : : "i"(AXPIKE_SYSNUM) : "a0","a7")
#define axpike_bnewsection() axpike_newsection()

#endif // _AXPIKE_IFACE_H_
