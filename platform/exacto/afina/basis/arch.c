/**
 * @file
 * @brief
 *
 * @author  Anton Kozlov
 * @date    30.10.2014
 */

#include <assert.h>
#include <stdint.h>

#include <hal/arch.h>
#include <hal/clock.h>
#include <hal/ipl.h>
#include <framework/mod/options.h>
#include <module/embox/arch/system.h>

#include <kernel/time/clock_source.h>
#include <kernel/time/time.h>
#include <kernel/time/timer.h>

#include <stm32f7xx_hal.h>

void HAL_MspInit(void)
{
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_RCC_SYSCFG_CLK_ENABLE();
}

static void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 216;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
	arch_shutdown(ARCH_SHUTDOWN_MODE_HALT);
  }

  

  /* activate the OverDrive to reach the 216 Mhz Frequency */
  if(HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
	arch_shutdown(ARCH_SHUTDOWN_MODE_HALT);
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
	arch_shutdown(ARCH_SHUTDOWN_MODE_HALT);
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_SDMMC1
                              |RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;  
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48SOURCE_PLL;
  PeriphClkInitStruct.Sdmmc1ClockSelection = RCC_SDMMC1CLKSOURCE_CLK48;
  
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
	arch_shutdown(ARCH_SHUTDOWN_MODE_HALT);
  }
}

void arch_init(void) {
	ipl_t ipl = ipl_save();

	static_assert(OPTION_MODULE_GET(embox__arch__system, NUMBER, core_freq) == 216000000);

	SystemInit();
	HAL_Init();

	SystemClock_Config();

	ipl_restore(ipl);
}

void arch_idle(void) {
	extern const struct clock_source *cs_jiffies;
	clock_t next_event_ticks, sleep_ticks;
	uint64_t next_event_cycles;
	struct clock_source *cs = (struct clock_source *) cs_jiffies;

	if (!cs) {
		return;
	}

	if (timer_strat_get_next_event(&next_event_ticks) != 0) {
		/* Sleep as long as possible */
		next_event_ticks = UINT32_MAX;
	} else {
		next_event_ticks = next_event_ticks - clock_sys_ticks();
	}

	next_event_cycles = clock_source_ticks2cycles(cs, next_event_ticks);
	if (next_event_cycles > cs->counter_device->mask) {
		next_event_cycles = cs->counter_device->mask;
	}

	clock_source_set_oneshot(cs);
	clock_source_set_next_event(cs, next_event_cycles);

	__asm__ ("wfi");

	sleep_ticks = clock_source_cycles2ticks(cs, clock_source_get_cycles(cs));

	jiffies_update(sleep_ticks);

	clock_source_set_periodic(cs, cs->event_device->event_hz);
}

void arch_shutdown(arch_shutdown_mode_t mode) {
	switch (mode) {
	case ARCH_SHUTDOWN_MODE_HALT:
	case ARCH_SHUTDOWN_MODE_REBOOT:
	case ARCH_SHUTDOWN_MODE_ABORT:
	default:
		HAL_NVIC_SystemReset();
		break;
	}

	/* NOTREACHED */
	while(1) {

	}
}

uint32_t HAL_GetTick(void) {
	return clock_sys_ticks();
}
