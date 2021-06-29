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
    buffer->str = (buffer->str + 1) & buffer->mask;
    if(buffer->str == buffer->lst)  
    {
        setemp_exbu8(buffer);   
    }
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
        buffer->lst = (buffer->lst + 1) & buffer->mask;
    }
    else 
    {
        if(buffer->lst == buffer->str) 
        {
            buffer->str = (buffer->str + 1) & buffer->mask;
        }
        buffer->lst = (buffer->lst + 1) & buffer->mask;
    }
}
uint8_t pshsft_exbu8(ExactoBufferUint8Type * buffer,const uint8_t value)
{
    if(!buffer->isExist)     
        return 0;
    uint16_t nxt = (buffer->lst + 1) & buffer->mask;
    if(nxt == buffer->str) 
        return 0;
    buffer->data[buffer->lst] = value;
	if(buffer->isEmpty)	
        buffer->isEmpty = 0;
    buffer->lst = nxt;
    return 1;
}
uint8_t grball_exbu8(ExactoBufferUint8Type * buffer, uint8_t * dst)
{
    if(!buffer->isExist || buffer->isEmpty)     return 0;
    uint16_t i = 0, adr = buffer->str;
    do
    {	
        adr = (buffer->str + i) & buffer->mask;
        dst[i] = buffer->data[adr];
        i++;
    }
	while(adr != buffer->lst);
    return 1;
}
uint8_t grbsvr_exbu8(ExactoBufferUint8Type * buffer, uint8_t * dst, const uint16_t length)
{
    if(!buffer->isExist || buffer->isEmpty)     return 0;
    uint16_t i = 0, adr, pt;
    uint16_t trg_len = getlen_exbu8(buffer);
    if (trg_len > length)
        trg_len = length;
    pt = (buffer->str + length) & buffer->mask;
    do
    {	
        adr = (buffer->str + i) & buffer->mask;
        dst[i] = buffer->data[adr];
        i++;
    }
    while(adr != pt);
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
uint16_t getlen_exbu8(ExactoBufferUint8Type * buffer)
{
    if( buffer->lst > buffer->str)
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
uint8_t watchsvr_exbu8( ExactoBufferUint8Type * buffer, uint8_t * dst, const uint16_t length_back )
{
    if(!buffer->isExist || buffer->isEmpty)     return 0;
    uint16_t i = 0, adr = buffer->lst;
    uint16_t trg_len = getlen_exbu8(buffer);
    if (trg_len > length_back)
        trg_len = length_back;
    do
	{	
        adr = (buffer->lst - trg_len + i) & buffer->mask;
        dst[i] = buffer->data[adr];
        i++;
	}
	while(adr != buffer->lst);
    return 1;
}
