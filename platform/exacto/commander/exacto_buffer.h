#ifndef EXACTO_BUFFER_H_
#define EXACTO_BUFFER_H_

#include <stdint.h> 
#include <stdlib.h>


#include <framework/mod/options.h>
#include <module/exacto/commander/buffer.h>
#define MODOPS_BUFFER_SZ OPTION_MODULE_GET(exacto__commander__buffer,NUMBER, buffersz)

#if MODOPS_BUFFER_SZ == 2
#define EXACTO_BUFFER_UINT8_SZ  2 
#elif MODOPS_BUFFER_SZ == 4
#define EXACTO_BUFFER_UINT8_SZ  4 
#elif MODOPS_BUFFER_SZ == 8
#define EXACTO_BUFFER_UINT8_SZ  8 
#elif MODOPS_BUFFER_SZ == 16
#define EXACTO_BUFFER_UINT8_SZ  16 
#elif MODOPS_BUFFER_SZ == 32
#define EXACTO_BUFFER_UINT8_SZ  32 
#elif MODOPS_BUFFER_SZ == 64
#define EXACTO_BUFFER_UINT8_SZ  64 
#elif MODOPS_BUFFER_SZ == 128
#define EXACTO_BUFFER_UINT8_SZ  128 
#elif MODOPS_BUFFER_SZ == 256
#define EXACTO_BUFFER_UINT8_SZ  256 
#elif MODOPS_BUFFER_SZ == 512
#define EXACTO_BUFFER_UINT8_SZ  512 
#elif MODOPS_BUFFER_SZ == 1024
#define EXACTO_BUFFER_UINT8_SZ  1024 
#elif MODOPS_BUFFER_SZ == 2048
#define EXACTO_BUFFER_UINT8_SZ  2048 
#elif MODOPS_BUFFER_SZ == 4096
#define EXACTO_BUFFER_UINT8_SZ  4096 
#elif MODOPS_BUFFER_SZ == 8192
#define EXACTO_BUFFER_UINT8_SZ  8192 
#else
#error Unsupported exacto buffer sz
#endif


typedef struct{
    uint8_t data[EXACTO_BUFFER_UINT8_SZ];
    uint16_t str;
    uint16_t lst;
    uint16_t mask;
    uint16_t datalen;
    uint8_t isEmpty;
    uint8_t isExist;
} ExactoBufferUint8Type;


#define __static_inline static inline

    
/**
 * @brief      { инициализируем буффер для заданного указателя }
 *
 * @param      buffer  указатель
 * @param[in]  sz      размер буффера
 */
extern int  setini_exbu8(ExactoBufferUint8Type * buffer);
/**
 * @brief      { получаем размер буффера заданного указателя }
 *
 * @param      buffer  указатель на буффер
 *
 * @return     размер буфферв
 */
extern uint8_t getlen_exbu8(ExactoBufferUint8Type * buffer);
/**
 * @brief      { забираем значение из начала буффера }
 *
 * @param      buffer  указатель на буффер
 * @param      fstval  переменная для записи первого значения
 *
 * @return     успешность операции
 */
extern uint8_t grbfst_exbu8(ExactoBufferUint8Type * buffer, uint8_t * fstval);

/**
 * @brief      { вставляем значение в конец буффера }
 *
 * @param      buffer  указатель на буффер
 * @param[in]  value   переменная, которая вставляется в буффер
 */
extern void pshfrc_exbu8(ExactoBufferUint8Type * buffer,const uint8_t value);

/**
 * @brief      { забираем данные из буффера в массив }
 *
 * @param      buffer  указатель на буффер источника данных
 * @param      dst     указатель на целевой массив
 *
 * @return     успешна ли операция
 */
extern uint8_t grball_exbu8(ExactoBufferUint8Type * buffer, uint8_t * dst);

/**
 * @brief      { убираем значение из начала буффера }
 *
 * @param      buffer  указатель на буффер
 *
 * @return     успешна ли операция
 */
extern uint8_t clrval_exbu8(ExactoBufferUint8Type * buffer);
/**
 * @brief      { убираем несколько значений из начала буффера, в случае, если данных в буффере меньше, удаляются только они }
 *
 * @param      buffer  указатель на буффер
 * @param[in]  cnt     количество данных для удаления
 *
 * @return     успешна ли операция
 */
extern uint8_t clrsvr_exbu8(ExactoBufferUint8Type * buffer, const uint8_t cnt);
/**
 * @brief      { очищаем буффер }
 *
 * @param      указатель на буффер
 *
 * @return     успешна ли операция
 */
extern uint8_t setemp_exbu8 (ExactoBufferUint8Type * buffer);
extern uint8_t mvbckone_exbu8( ExactoBufferUint8Type * buffer );
extern uint8_t mvbcksvr_exbu8( ExactoBufferUint8Type * buffer, const uint16_t length_back );


#endif /* EXACTO_BUFFER_H_ */
