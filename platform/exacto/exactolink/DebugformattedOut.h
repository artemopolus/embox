/**
 * @file DebugformattedOut.h
 * @author GubskiyAS (GubskiyAS@mail.ru)
 * @brief Заголовный файл форматного вывода
 * @version 0.1
 * @date 2020-11-24
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef DEBUG_FORMATTED_OUT_H
#define DEBUG_FORMATTED_OUT_H

// #include "ff.h"
// #include "ff_gen_drv.h"
// #include "user_diskio.h"
// #include "fatfs.h"

#define LENGTH_DEBUG_INFO_ARRAY 6000 ///< Размер массива для вывода (должен быть выставлен больше максимального количества символов за один запуск formattedOut)



// extern FIL file_debug_out_bus; ///< Файловый обьект для файла потока с измерительной шины
extern void formattedOut(char * preString, uint8_t * ptr, uint32_t size);
extern void formattedOutIntVariable(char * preString, uint32_t ptr);

#endif
