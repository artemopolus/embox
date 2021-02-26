#ifndef EXACTO_BUFFER_H_
#define EXACTO_BUFFER_H_

#include <stdint.h> 
#include <stdlib.h>


typedef struct{
    uint8_t * data;
    uint8_t str;
    uint8_t lst;
    uint8_t isExist;
    uint8_t datalen;
    uint8_t isEmpty;
    uint8_t mask;
} ExactoBufferUint8Type;

typedef enum{
    EXACTO_BUFLEN_02 = 2,
    EXACTO_BUFLEN_04 = 4,
    EXACTO_BUFLEN_08 = 8,
    EXACTO_BUFLEN_16 = 16,
    EXACTO_BUFLEN_64 = 64,
    EXACTO_BUFLEN_128 = 128,
    EXACTO_BUFLEN_256 = 256
}ExactoBufferUint8TypSizes;

#define __static_inline static inline
    
/**
 * @brief      { инициализируем буффер для заданного указателя }
 *
 * @param      buffer  указатель
 * @param[in]  sz      размер буффера
 */
__static_inline void    setini_exbu8(ExactoBufferUint8Type * buffer, ExactoBufferUint8TypSizes sz)
{
    buffer->str = 0;
    buffer->lst = 0;
    buffer->data = (uint8_t *)calloc(sz, sizeof(uint8_t));
    buffer->isExist = 1;
    buffer->isEmpty = 1;
    buffer->datalen = sz;
    buffer->mask = sz - 1;
}
/**
 * @brief      { получаем размер буффера заданного указателя }
 *
 * @param      buffer  указатель на буффер
 *
 * @return     размер буфферв
 */
__static_inline uint8_t getlen_exbu8(ExactoBufferUint8Type * buffer)
{
    if( buffer->lst >= buffer->str)
        return (buffer->lst - buffer->str);
    else
        return (buffer->lst + buffer->datalen - buffer->str);
}
/**
 * @brief      { забираем значение из начала буффера }
 *
 * @param      buffer  указатель на буффер
 * @param      fstval  переменная для записи первого значения
 *
 * @return     успешность операции
 */
__static_inline uint8_t grbfst_exbu8(ExactoBufferUint8Type * buffer, uint8_t * fstval)
{
    if(!buffer->isExist || buffer->isEmpty)     return 0;
    *fstval = buffer->data[buffer->str];
    if(buffer->str == buffer->lst)  {buffer->isEmpty = 1;   return 1;}
    buffer->str = (buffer->str + 1) & buffer->mask;
    return 1;
}
/**
 * @brief      { вставляем значение в конец буффера }
 *
 * @param      buffer  указатель на буффер
 * @param[in]  value   переменная, которая вставляется в буффер
 */
__static_inline void pshfrc_exbu8(ExactoBufferUint8Type * buffer,const uint8_t value)
{
    if(!buffer->isExist)     return;
    buffer->data[buffer->lst] = value;
		if(buffer->isEmpty)	buffer->isEmpty = 0;
    else 
		{
			buffer->lst = (buffer->lst + 1) & buffer->mask;
			if(buffer->lst == buffer->str) buffer->str = (buffer->str + 1) & buffer->mask;
		}
}
/**
 * @brief      { забираем данные из буффера в массив }
 *
 * @param      buffer  указатель на буффер источника данных
 * @param      dst     указатель на целевой массив
 *
 * @return     успешна ли операция
 */
__static_inline uint8_t grball_exbu8(ExactoBufferUint8Type * buffer, uint8_t * dst)
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
/**
 * @brief      { убираем значение из начала буффера }
 *
 * @param      buffer  указатель на буффер
 *
 * @return     успешна ли операция
 */
__static_inline uint8_t clrval_exbu8(ExactoBufferUint8Type * buffer)
{
    if(!buffer->isExist || buffer->isEmpty)     return 0;
    if(buffer->str == buffer->lst)  {buffer->isEmpty = 1;   return 0;}
    buffer->str = (buffer->str + 1) & buffer->mask;
    return 1;
}
/**
 * @brief      { убираем несколько значений из начала буффера, в случае, если данных в буффере меньше, удаляются только они }
 *
 * @param      buffer  указатель на буффер
 * @param[in]  cnt     количество данных для удаления
 *
 * @return     успешна ли операция
 */
__static_inline uint8_t clrsvr_exbu8(ExactoBufferUint8Type * buffer, const uint8_t cnt)
{
    if(!buffer->isExist || buffer->isEmpty)     return 0;
    uint8_t i = 0;
    while(clrval_exbu8(buffer) &&(i < cnt))
    {
        i++;
    }
    return 1;
}

/**
 * @brief      { очищаем буффер }
 *
 * @param      указатель на буффер
 *
 * @return     успешна ли операция
 */
__static_inline uint8_t setemp_exbu8 (ExactoBufferUint8Type * buffer)
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

#endif /* EXACTO_BUFFER_H_ */
