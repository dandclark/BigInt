/*******************************************************************************
*    No Copyright - this is freeware
********************************************************************************

     File:	CMemLeak.c     

     Author:    Xie Wei Bao (UK) Ltd

     email:     tech@cup.btinternet.co.uk

     Purpose:   Detecting memory leaks

********************************************************************************
*/

#include "CMemLeak.h"
#undef malloc
#undef realloc
#undef free
#undef strdup
#undef calloc
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

/* Guards for checking illegal memory writes */
static const char xwbProtect[] = "DeAd";
static const unsigned int xwbProtSize = sizeof (xwbProtect);

/* Filename of report file */
static const char xwbReportFilename[] = "CMemLeak.txt";

/* Uninitialized memory - pick a value that will cause the most problems */
static const unsigned char xwbUninit = 0x55;

/* Clean memory - pick a value which will cause the most problems */
static const unsigned char xwbFreed = 0xAA;

static const char xwbIMW[] = "IMW";	/* Illegal memory write */
static const char xwbMLK[] = "MLK";     /* Memory leak */
static const char xwbFNH[] = "FNH";     /* Free Non Heap memory */
static const char xwbFMW[] = "FMW";     /* Free Memory Write */

/* Node for storing the allocation details */
struct XWBNode
{
    struct XWBNode* mPrev;
    struct XWBNode* mNext;
    void* mPtr;
    unsigned int mSize;
    const char* mFile;
    unsigned int mLine;
    const char* mName;
};

struct XWBList
{
    /* Doubly linked list */
    struct XWBNode* mHead;
    struct XWBNode* mTail;

    FILE* mReport;
    unsigned long mAllocUsed;           /* Max in the life of the program */
    unsigned long mAllocTotal;          /* Number of allocations */
    unsigned long mAllocCurrent;        /* Current allocation */

    unsigned int mFree;                 /* 1 if memory to be freed */
    unsigned int mAllocMax;             /* Not yet - preallocate nodes */
    struct XWBNode* mNode;              /* Not yet - contiguous node storage */
    struct XWBNode* mUnused;            /* Not yet - chain of free nodes */
};


/* Link for storing allocation details */
static struct XWBList xwbMem = 
{
    (struct XWBNode*) 0,
    (struct XWBNode*) 0
};
/*******************************************************************************
* Forward declarations
*******************************************************************************/
static struct XWBNode* XWBNodeNew (void);
static void XWBNodeDelete (struct XWBNode* that);
static void XWBNodeFree (
    struct XWBNode* that,
    const char* iName,
    const char* iFile,
    const unsigned int iLine);
static void XWBNodeLink (
    struct XWBNode*,
    struct XWBNode*,
    struct XWBNode*);
static void XWBNodeSet (
    struct XWBNode* that,
    void* iPtr,
    const unsigned int iSize,
    const char* iFile,
    const unsigned int iLine);
/* static void XWBNodeIMWCheck (struct XWBNode* that); */
static void XWBMemNew (void);
static struct XWBNode* XWBMemFind (
    void* iPtr,
    unsigned int* oSIze,
    const char** oFile,
    unsigned int* oLine);
static void XWBMemDump (void);
static void XWBMemInsert (
    void* iPtr,
    const unsigned int iSize,
    const char* iFile,
    const unsigned int iLine);
/*******************************************************************************
* New node
*******************************************************************************/
static struct XWBNode* XWBNodeNew (void)
{
    struct XWBNode* that = (struct XWBNode*) malloc (sizeof (struct XWBNode));
    that->mPrev = 0;
    that->mNext = 0;
    that->mName = 0;

    return that;
}
/*******************************************************************************
* Delete node
*******************************************************************************/
static void XWBNodeDelete (struct XWBNode* that)
{
    /* Unlink */
    if (that->mPrev)
        that->mPrev->mNext = that->mNext;
	    
    if (that->mNext)
        that->mNext->mPrev = that->mPrev;
	    
    free (that);
}
/*******************************************************************************
* Free a node
*******************************************************************************/
static void XWBNodeFree (
    struct XWBNode* that,
    const char* iName,
    const char* iFile,
    const unsigned int iLine)
{
    that->mFile = iFile;
    that->mLine = iLine;
    that->mName = iName;
}
/*******************************************************************************
* Link up node
*******************************************************************************/
static void XWBNodeLink (
    struct XWBNode* that,
    struct XWBNode* iPrev,
    struct XWBNode* iNext
)
{
    that->mPrev = iPrev;
    if (iPrev != 0)
        iPrev->mNext = that;
        
    that->mNext = iNext;
    if (iNext != 0)
        iNext->mPrev = that;
}
/*******************************************************************************
* Set up node
*******************************************************************************/
static void XWBNodeSet (
    struct XWBNode* that,
    void* iPtr,
    const unsigned int iSize,
    const char* iFile,
    const unsigned int iLine
)
{
    that->mPtr  = iPtr;
    that->mSize = iSize;
    that->mFile = iFile;
    that->mLine = iLine;
}
/*******************************************************************************
* Initialization
*******************************************************************************/
static void XWBMemNew (void)
{
    /* Set up the doubly linked list */
    xwbMem.mHead = XWBNodeNew ();
    xwbMem.mTail = XWBNodeNew ();
    XWBNodeLink (xwbMem.mHead, 0, xwbMem.mTail);
    XWBNodeLink (xwbMem.mTail, xwbMem.mHead, 0);

    /* Initialize statistics */
    xwbMem.mAllocUsed = 0L;
    xwbMem.mAllocTotal = 0L;
    xwbMem.mAllocCurrent = 0L;

    xwbMem.mFree = 1;

    xwbMem.mReport = fopen (xwbReportFilename, "w");

    atexit (XWBReportFinal);
}
/*******************************************************************************
* Dump List - used for debugging only
*******************************************************************************/
static void XWBMemDump ()
{
    int count;
    struct XWBNode* iter = xwbMem.mHead;
    
    for (count = 0; iter != 0; count++, iter = iter->mNext)
    {
        fprintf (xwbMem.mReport, "%d node %p prev %p next %p\n", count, iter, iter->mPrev, iter->mNext);
    }
    fprintf (xwbMem.mReport, "\n");
}
/*******************************************************************************
* Insert into the tracking list
*******************************************************************************/
void XWBMemInsert (
    void* iPtr,
    const unsigned int iSize, 
    const char* iFile, 
    const unsigned int iLine
)
{
    struct XWBNode* node;
    if (xwbMem.mHead == 0)
    {
        XWBMemNew ();
    }

    /* Link in the new node */
    node = XWBNodeNew ();
    XWBNodeSet (node, iPtr, iSize, iFile, iLine);
    XWBNodeLink (node, xwbMem.mTail->mPrev, xwbMem.mTail);

    xwbMem.mAllocTotal   += 1;
    xwbMem.mAllocCurrent += iSize;
    if (xwbMem.mAllocUsed < xwbMem.mAllocCurrent)
        xwbMem.mAllocUsed = xwbMem.mAllocCurrent;
}
/*******************************************************************************
* Find a memory pointer
*******************************************************************************/
static struct XWBNode* XWBMemFind (
    void* iPtr,
    unsigned int* oSize, 
    const char** oFile, 
    unsigned int* oLine
)
{
    struct XWBNode* result = 0;
    struct XWBNode* iter;
    
    iter = xwbMem.mTail;
    while ((iter = iter->mPrev) != xwbMem.mHead)
    {
        if (iter->mPtr == iPtr)
        {
            result = iter;
            *oSize = iter->mSize;
            *oFile = iter->mFile;
            *oLine = iter->mLine;
            break;
        }
    }
    return result;
}
/*******************************************************************************
* Allocate memory
*******************************************************************************/
void* XWBMalloc (unsigned int iSize, const char* iFile, const unsigned int iLine)
{
    register unsigned int usize;
    unsigned char* result;
    
    usize = ((iSize + xwbProtSize) / sizeof (unsigned int) + 1) * sizeof (unsigned int);
    result = malloc (usize);
    memset (result, xwbUninit, usize);
    memcpy (&result[iSize], xwbProtect, xwbProtSize);
    
    XWBMemInsert (result, iSize, iFile, iLine);
    return (void*) result;
}
/*******************************************************************************
* Allocate memory
*******************************************************************************/
void* XWBRealloc (void* iPtr, unsigned int iSize, const char* iFile, const unsigned int iLine)
{
    register unsigned int usize;
    unsigned char* result;
    struct XWBNode* node;
    unsigned int size, line;
    const char* name;
    
    usize = ((iSize + xwbProtSize) / sizeof (unsigned int) + 1) * sizeof (unsigned int);
    result = realloc (iPtr, usize);
    /* memset (result, xwbUninit, usize); */
    memcpy (&result[iSize], xwbProtect, xwbProtSize);
    
    /* Update the allocation details */
    name = iFile;
    line = iLine;
    node = XWBMemFind (iPtr, &size, &name, &line);
    XWBNodeSet (node, result, iSize, name, line);

    xwbMem.mAllocCurrent -= size;
    xwbMem.mAllocCurrent += iSize;
    if (xwbMem.mAllocUsed < xwbMem.mAllocCurrent)
        xwbMem.mAllocUsed = xwbMem.mAllocCurrent;
    return (void*) result;
}
/*******************************************************************************
* Unallocate memory
*******************************************************************************/
void  XWBFree (void* iPtr, const char* iDesc, const char* iFile, const unsigned int iLine)
{
    /* Check if it is one of ours */
    const char* file;
    unsigned int line;
    unsigned int size;
    struct XWBNode* node;

    node = XWBMemFind (iPtr, &size, &file, &line);
    if (node != 0)
    {
        unsigned char* ptr = (unsigned char*) iPtr;
        if (memcmp (&ptr[size], xwbProtect, xwbProtSize) != 0)
        {
            /* Illegal memory write */
            fprintf (xwbMem.mReport, "%s: %s allocated %s: %u\n", xwbIMW, iDesc, file, line);
            fprintf (xwbMem.mReport, "   : %s deallocated %s: %u\n", iDesc, iFile, iLine); 
        }
        memset (iPtr, xwbFreed, size);
        if (xwbMem.mFree)
        {
            free (iPtr);
            XWBNodeDelete (node);
        }
        else
        {
            /* Save the freed memory details */
            XWBNodeFree (node, iDesc, iFile, iLine);
        }
        xwbMem.mAllocCurrent -= size;
    }
    else
    {
        /* Free non-heap memory */
        fprintf (xwbMem.mReport, "%s: %s deallocated %s: %u\n", xwbFNH, iDesc, iFile, iLine);
        
        /* Don't do it otherwise it might crash */
    }
}
/*******************************************************************************
* Do not free
*******************************************************************************/
void XWBNoFree (void)
{
    if (xwbMem.mHead == 0)
    {
        XWBMemNew ();
    }
    xwbMem.mFree = 0;
}
/*******************************************************************************
* Report
*******************************************************************************/
void  XWBReport (const char* iTag)
{
    struct XWBNode* iter;
    unsigned char* ptr;
    unsigned int size;
    register unsigned int u;
    
    if (xwbMem.mHead == 0)
    {
        XWBMemNew ();
    }

    if (iTag)
        fprintf (xwbMem.mReport, "\n%s\n", iTag);

    /* XWBListDump (); */
    iter = xwbMem.mHead;    
    while ((iter = iter->mNext) != xwbMem.mTail)
    {
        ptr = (unsigned char*) iter->mPtr;
        size  = iter->mSize;
        if (iter->mName)
        {
            /* Check that there are no FMWs */
            for (u = 0; u < size; u++)
            {
                if (ptr[u] != xwbFreed)
                {
                    fprintf (xwbMem.mReport, "%s: %s freed at %s: %u\n",
                        xwbFMW, iter->mName, iter->mFile, iter->mLine);
                    break;
                }
            }
        }
        else
        {
            fprintf (xwbMem.mReport, "%s: %p %u bytes allocated %s: %u\n", 
                xwbMLK, iter->mPtr, iter->mSize, iter->mFile, iter->mLine);
            if (memcmp (&ptr[size], xwbProtect, xwbProtSize) != 0)
            {
                /* Illegal memory write */
                fprintf (xwbMem.mReport, "%s: %p allocated %s: %u\n", 
                    xwbIMW, ptr, iter->mFile, iter->mLine);
            }
        }
    }

    /* Print statistics */
    fprintf (xwbMem.mReport, "Total allocations    : %ld\n",
        xwbMem.mAllocTotal);
    fprintf (xwbMem.mReport, "Max memory allocation: %ld (%ldK)\n", 
        xwbMem.mAllocUsed, xwbMem.mAllocUsed / 1024);
    fprintf (xwbMem.mReport, "Total leak           : %ld\n\n", 
        xwbMem.mAllocCurrent);
}

/*******************************************************************************
* Final Report
*******************************************************************************/
void  XWBReportFinal (void)
{
    XWBReport ("Final Report");
    fclose (xwbMem.mReport);
    xwbMem.mReport = 0;
}

/*******************************************************************************
* Duplicate a string
*******************************************************************************/
char* XWBStrDup (const char* iOrig, const char* iFile, const unsigned int iLine)
{
    char* result;
    result = XWBMalloc (strlen (iOrig) + 1, iFile, iLine);
    strcpy (result, iOrig);
    return result;
}
/*******************************************************************************
* Allocate a number of items of a specified size
*******************************************************************************/
void* XWBCalloc (unsigned int iNum, unsigned int iSize, const char* iFile, const unsigned int iLine)
{
    void* result;
    unsigned int actual = (((iSize - 1)/sizeof(int)) + 1) * sizeof (int) * iNum;
    result = XWBMalloc (actual, iFile, iLine);
    memset (result, 0, actual);
    return result;
}
