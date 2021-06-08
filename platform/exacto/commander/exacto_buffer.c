#include <commander/exacto_buffer.h>

int setini_exbu8(ExactoBufferUint8Type * buffer)
{
    buffer->str = 0;
    buffer->lst = 0;
    buffer->isExist = 1;
    buffer->isEmpty = 1;
    buffer->datalen = EXACTO_BUFFER_UINT8_SZ;
    buffer->mask = EXACTO_BUFFER_UINT8_SZ - 1;
    return 0;
}
uint8_t grbfst_exbu8(ExactoBufferUint8Type * buffer, uint8_t * fstval)
{
    if(!buffer->isExist || buffer->isEmpty)     return 0;
    *fstval = buffer->data[buffer->str];
    if(buffer->str == buffer->lst)  {buffer->isEmpty = 1;   return 1;}
    buffer->str = (buffer->str + 1) & buffer->mask;
    return 1;
}
void pshfrc_exbu8(ExactoBufferUint8Type * buffer,const uint8_t value)
{
    if(!buffer->isExist)     
        return;
    buffer->data[buffer->lst] = value;
	if(buffer->isEmpty)	
    {
        buffer->isEmpty = 0;
    }
    //else 
    //{
        buffer->lst = (buffer->lst + 1) & buffer->mask;
        if(buffer->lst == buffer->str) 
        {
            buffer->str = (buffer->str + 1) & buffer->mask;
        }
    //}
}
uint8_t grball_exbu8(ExactoBufferUint8Type * buffer, uint8_t * dst)
{
    if(!buffer->isExist || buffer->isEmpty)     return 0;
    uint8_t i = 0, adr = buffer->str;
    do
		{	
			adr = (buffer->str + i) & buffer->mask;
			dst[i] = buffer->data[adr];
			i++;
		}
		while(adr != buffer->lst);
    return 1;
}
uint8_t clrval_exbu8(ExactoBufferUint8Type * buffer)
{
    if(!buffer->isExist || buffer->isEmpty)     return 0;
    if(buffer->str == buffer->lst)  {buffer->isEmpty = 1;   return 0;}
    buffer->str = (buffer->str + 1) & buffer->mask;
    return 1;
}
uint8_t clrsvr_exbu8(ExactoBufferUint8Type * buffer, const uint8_t cnt)
{
    if(!buffer->isExist || buffer->isEmpty)     return 0;
    uint8_t i = 0;
    while(clrval_exbu8(buffer) &&(i < cnt))
    {
        i++;
    }
    return 1;
}
uint8_t setemp_exbu8 (ExactoBufferUint8Type * buffer)
{
    if (!buffer->isExist)
        return 0;
    if (buffer->isEmpty)
        return 1;
    buffer->lst = 0;
    buffer->str = 0;
    buffer->isEmpty = 1;
    return 1;
}
uint8_t getlen_exbu8(ExactoBufferUint8Type * buffer)
{
    if( buffer->lst >= buffer->str)
        return (buffer->lst - buffer->str);
    else
        return (buffer->lst + buffer->datalen - buffer->str);
}
uint8_t mvbckone_exbu8( ExactoBufferUint8Type * buffer )
{
    if ( !buffer->isExist || buffer->isEmpty )     return 0;
    buffer->str = (buffer->str - 1) & buffer->mask;
    if (buffer->lst == buffer->str)
    {
        buffer->str = (buffer->str + 1) & buffer->mask;
        return 0;
    }
    return 1;
}
uint8_t mvbcksvr_exbu8( ExactoBufferUint8Type * buffer, const uint16_t length_back )
{
    for (uint8_t i = 0; i < length_back; i++)
    {
        if (!mvbckone_exbu8(buffer))
        return 0;
    }
    return 1;
}
