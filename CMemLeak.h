/*******************************************************************************
*    No Copyright - this is freeware
********************************************************************************

     File:	CMemleak.h

     Author:    Xie Wei Bao (UK) Ltd

     email:     tech@cup.btinternet.co.uk

     Purpose:   Detecting memory leaks
    
********************************************************************************
*/
#ifndef memleak_h
#define memleak_h

/* Used for tracking allocations */
extern void* XWBMalloc (
    unsigned int iSize,
    const char* iFile,
    const unsigned int iLine);
extern void* XWBCalloc (
    unsigned int iNum,
    unsigned int iSize,
    const char* iFile,
    const unsigned int iLine);
extern char* XWBStrDup (
    const char* iOrig,
    const char* iFile,
    const unsigned int iLine);
/* Used for tracking reallocations */
extern void* XWBRealloc (
    void* iPrev,
    unsigned int iSize,
    const char* iFile,
    const unsigned int iLine);
/* Used for tracking deallocations */
extern void  XWBFree (
    void* iPtr,
    const char* iDesc,
    const char* iFile,
    const unsigned int iLine);
/* Used for reporting */
extern void  XWBReport (const char* iTag);
extern void  XWBReportFinal (void);
/* Used for detecting FMW */
extern void  XWBNoFree (void);
extern void  XWBPreallocate (const int iInitialAllocations);

#ifdef _DEBUG
#define malloc(x) XWBMalloc((x), __FILE__, __LINE__)
#define realloc(x,size) XWBRealloc(x,(size),__FILE__,__LINE__)
#define free(x)   XWBFree(x, #x, __FILE__, __LINE__)
#define strdup(x) XWBStrDup(x, __FILE__, __LINE__)
#define calloc(num,size) XWBCalloc((num), (size), __FILE__, __LINE__)
#endif

#endif

