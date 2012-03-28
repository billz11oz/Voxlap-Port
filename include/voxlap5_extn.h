extern float optistrx, optistry, optiheix, optiheiy, optiaddx, optiaddy;
extern int64_t foglut[2048], fogcol;

#ifndef _DOS
#ifdef __cplusplus
extern "C" {
#endif
extern void *opti4asm;
#define opti4 ((point4d *)&opti4asm)
#ifdef __cplusplus
}
#endif
#endif