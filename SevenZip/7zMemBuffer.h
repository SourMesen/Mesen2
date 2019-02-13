#pragma once

#include "7zTypes.h"

EXTERN_C_BEGIN

/* ---------- File ---------- */

typedef struct
{
	void* buffer;
	Int64 size;
	Int64 pos;
} CSzMemBuffer;

/* reads max(*size, remain file's size) bytes */
WRes MemBuffer_Read(CSzMemBuffer *p, void *data, size_t *size);
WRes MemBuffer_Seek(CSzMemBuffer *p, Int64 *pos, ESzSeek origin);

/* ---------- FileInStream ---------- */
typedef struct
{
  ISeekInStream s;
  CSzMemBuffer buffer;
} CMemBufferInStream;

void MemBufferInit(CMemBufferInStream *memBuferStream, CLookToRead *lookStream, void* buffer, size_t size);

EXTERN_C_END
