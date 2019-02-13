#include "Precomp.h"
#include "7zMemBuffer.h"
#include <memory.h>

WRes MemBuffer_Read(CSzMemBuffer *p, void *data, size_t *size)
{
	size_t originalSize = *size;
	if(originalSize == 0)
		return 0;

	size_t length = (size_t)(p->pos + (Int64)(*size) > p->size ? p->size - p->pos - 1 : *size);
	memcpy(data, (char*)(p->buffer) + p->pos, length);
	p->pos += length;
	return 0;
}

WRes MemBuffer_Seek(CSzMemBuffer *p, Int64 *pos, ESzSeek origin)
{
	switch(origin) {
		case SZ_SEEK_SET: p->pos = 0 + *pos; break;
		case SZ_SEEK_CUR: p->pos += *pos; break;
		case SZ_SEEK_END: p->pos = p->size - *pos; break;
		default: return 1;
	}
	*pos = p->pos;
	return 0;
}

static SRes MemBufferInStream_Read(void *pp, void *buf, size_t *size)
{
	CMemBufferInStream *p = (CMemBufferInStream *)pp;
	return (MemBuffer_Read(&p->buffer, buf, size) == 0) ? SZ_OK : SZ_ERROR_READ;
}

static SRes MemBufferInStream_Seek(void *pp, Int64 *pos, ESzSeek origin)
{
	CMemBufferInStream *p = (CMemBufferInStream *)pp;
	return MemBuffer_Seek(&p->buffer, pos, origin);
}

void MemBufferInit(CMemBufferInStream *memBuferStream, CLookToRead *lookStream, void* buffer, size_t size)
{
	memBuferStream->buffer.buffer = buffer;
	memBuferStream->buffer.pos = 0;
	memBuferStream->buffer.size = size;

	memBuferStream->s.Read = MemBufferInStream_Read;
	memBuferStream->s.Seek = MemBufferInStream_Seek;

	LookToRead_CreateVTable(lookStream, False);
	lookStream->realStream = &memBuferStream->s;
	LookToRead_Init(lookStream);
}