/*
 * guzhoudiaoke@126.com
 * 2018-03-05
 */

#define _AUPBND         (sizeof (uint32) - 1)
#define _ADNBND         (sizeof (uint32) - 1)
typedef char* va_list;
#define _bnd(X, bnd)    (((sizeof (X)) + (bnd)) & (~(bnd)))
#define va_arg(ap, T)   (*(T *)(((ap) += (_bnd (T, _AUPBND))) - (_bnd (T,_ADNBND))))
#define va_end(ap)      (void) 0
#define va_start(ap, A) (void) ((ap) = (((char *) &(A)) + (_bnd (A,_AUPBND))))
