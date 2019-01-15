/*

Copyright 2011-2018 Stratify Labs, Inc

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	 http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include <string.h>
#include <fcntl.h>
#include <sos/sos.h>
#include <mcu/debug.h>
#include <cortexm/task.h>
#include <cortexm/mpu.h>
#include <sos/link/types.h>
#include <mcu/emc.h>
#include "board_config.h"

#define TRACE_COUNT 8
#define TRACE_FRAME_SIZE sizeof(link_trace_event_t)
#define TRACE_BUFFER_SIZE (sizeof(link_trace_event_t)*TRACE_COUNT)
static char trace_buffer[TRACE_FRAME_SIZE*TRACE_COUNT];
const ffifo_config_t board_trace_config = {
	.frame_count = TRACE_COUNT,
	.frame_size = sizeof(link_trace_event_t),
	.buffer = trace_buffer
};
ffifo_state_t board_trace_state;

extern void SystemClock_Config();
extern void configure_external_memory();
extern void LCD_Config();
extern const emc_config_t emc_sdram_config;

void board_trace_event(void * event){
	link_trace_event_header_t * header = event;
	devfs_async_t async;
	const devfs_device_t * trace_dev = &(devfs_list[0]);

	//write the event to the fifo
	memset(&async, 0, sizeof(devfs_async_t));
	async.tid = task_get_current();
	async.buf = event;
	async.nbyte = header->size;
	async.flags = O_RDWR;
	trace_dev->driver.write(&(trace_dev->handle), &async);
}

void board_event_handler(int event, void * args){
	devfs_handle_t emc_handle;
	switch(event){
		case MCU_BOARD_CONFIG_EVENT_ROOT_TASK_INIT:
			break;

		case MCU_BOARD_CONFIG_EVENT_ROOT_FATAL:
			//start the bootloader on a fatal event
			//mcu_core_invokebootloader(0, 0);
			if( args != 0 ){
				mcu_debug_log_error(MCU_DEBUG_SYS, "Fatal Error %s", (const char*)args);
			} else {
				mcu_debug_log_error(MCU_DEBUG_SYS, "Fatal Error unknown");
			}
			while(1){
				;
			}
			break;

		case MCU_BOARD_CONFIG_EVENT_ROOT_INITIALIZE_CLOCK:
			SystemClock_Config();

			configure_external_memory();

			emc_handle.port = 0;
			emc_handle.config = &emc_sdram_config;
			emc_handle.state = 0;

			//this will keep the SDRAM running if the application opens an instance of the device and then closes it
			mcu_emc_sdram_open(&emc_handle);

			memset((void*)0xC0000000, 0xC0, (480*272*2));

			LCD_Config();
			break;

		case MCU_BOARD_CONFIG_EVENT_START_INIT:
			break;

		case MCU_BOARD_CONFIG_EVENT_START_LINK:
			mcu_debug_log_info(MCU_DEBUG_USER1, "Start LED %d");
			sos_led_startup();
			break;

		case MCU_BOARD_CONFIG_EVENT_START_FILESYSTEM:
			break;
	}
}
