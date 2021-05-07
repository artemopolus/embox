/**
 * @file DebugformattedOut.c
 * @author GubskiyAS (GubskiyAS@mail.ru)
 * @brief Файл реализаций форматного вывода для отладки библиотеки EXACTOLINK
 * @version 0.1
 * @date 2020-11-24
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#include <stdio.h>
#include <stdint.h>
#include "string.h"

// #include "stm32f7xx_hal.h"
// #include "stm32f7xx_hal_def.h"
// #include "stm32f7xx_hal_sd.h"
// #include "sdmmc.h"
// #include "ff.h"
// #include "ff_gen_drv.h"
// #include "user_diskio.h"
// #include "fatfs.h"

#include "DebugformattedOut.h"

// FIL file_debug_out_bus; ///< Файловый обьект для форматного вывода


uint8_t out_text[LENGTH_DEBUG_INFO_ARRAY];///< промежуточный массив используется для подготовки форматного вывода в файл
uint64_t shift_position_on_file =0; //здесь хранится общее смещение по файлу file_debug_out_bus

/**
 * @brief функция форматного вывода байтового массива заданного размера в hex виде
 * 
 * @param preString - указатель на строку из масксимум 30 символов в ASCII кодировке. Вставляется перед байтовым массивом
 * @param ptr - указатель на начало области вывода (на байтовый массив)
 * @param size - размер области вывода в байтах
 */
void formattedOut(char * preString, uint8_t * ptr, uint32_t size)
{
	// volatile uint32_t length_to_write_SD = 0;//здесь будет находится количество символов в текущей строке
	// uint32_t cnt_writed_bytes_in_fact = 0; //количество фактически записанных байт в файл
	
	

	// if( (size + 30)*3 >= LENGTH_DEBUG_INFO_ARRAY) {while(1);/*return;*/}//30 - это фиксированный текст например "Pack Seti ->"
	// length_to_write_SD = sprintf((char *)out_text, "%s", (char *) preString);
	// for (uint32_t cnt=0;cnt<size; cnt++)
	// 	length_to_write_SD += sprintf((char *)out_text + length_to_write_SD, "%.2X ", ptr[cnt]);
	// length_to_write_SD += sprintf((char *)out_text + length_to_write_SD, "\n");
	

	// if(f_write(&file_debug_out_bus, out_text, length_to_write_SD, (void *)&cnt_writed_bytes_in_fact) == FR_OK)
	// {	
	// 	if (length_to_write_SD != cnt_writed_bytes_in_fact )  Error_Handler();
	// 	shift_position_on_file += length_to_write_SD;
	// 	f_lseek(&file_debug_out_bus, shift_position_on_file);//смещаем позицию для записи
	// }
	// else Error_Handler();
	return;
}

/**
 * @brief функция форматного вывода целочисенной переменной в десятичном виде с именем самой переменной в предстроке  
 * 
 * @param preString -  указатель на строку из максимум 50 символов с именем переменной в ASCII кодировке
 * @param value - значение переменной для вывода  
 */
void formattedOutIntVariable(char * preString, uint32_t  value)
{
	// volatile uint32_t length_to_write_SD = 0;//здесь будет находится количество символов в текущей строке
	// uint32_t cnt_writed_bytes_in_fact = 0; //количество фактически записанных байт в файл


	// length_to_write_SD = sprintf((char *)out_text, "%.50s = ", (char *) preString);
	// length_to_write_SD += sprintf((char *)out_text + length_to_write_SD, "%u \n",(unsigned int) value);


	// if(f_write(&file_debug_out_bus, out_text, length_to_write_SD, (void *)&cnt_writed_bytes_in_fact) == FR_OK)
	// {
	// 	if (length_to_write_SD != cnt_writed_bytes_in_fact )  Error_Handler();
	// 	shift_position_on_file += length_to_write_SD;
	// 	f_lseek(&file_debug_out_bus, shift_position_on_file);//смещаем позицию для записи
	// }
	// else Error_Handler();
	return;
}
