
#include <device/sys.h>
#include <mcu/debug.h>
#include <mcu/arch/stm32/stm32f7xx/stm32f7xx_hal.h>
#include "display_device.h"

extern DMA2D_HandleTypeDef hdma2d;


static int display_device_getinfo(const devfs_handle_t * handle, void * ctl);
static int display_device_clear(const devfs_handle_t * handle, void * ctl);
static int display_device_refresh(const devfs_handle_t * handle, void * ctl);
static int display_device_init(const devfs_handle_t * handle, void * ctl);
static int display_device_enable(const devfs_handle_t * handle, void * ctl);
static int display_device_disable(const devfs_handle_t * handle, void * ctl);
static int display_device_isbusy(const devfs_handle_t * handle, void * ctl);

int display_device_open(const devfs_handle_t * handle){
	display_device_init(handle,0);
	return SYSFS_RETURN_SUCCESS;
}

int display_device_close(const devfs_handle_t * handle){
	return SYSFS_RETURN_SUCCESS;
}

int display_device_ioctl(const devfs_handle_t * handle, int request, void * ctl){

	switch(request){
		case I_DISPLAY_GETVERSION: return DISPLAY_VERSION;
		case I_DISPLAY_GETINFO: return display_device_getinfo(handle, ctl);
		case I_DISPLAY_CLEAR: return display_device_clear(handle, ctl);
		case I_DISPLAY_REFRESH: return display_device_refresh(handle, ctl);
		//case I_DISPLAY_INIT: return display_device_init(handle, ctl);
		//case I_DISPLAY_ENABLE: return display_device_enable(handle, ctl);
		//case I_DISPLAY_DISABLE: return display_device_disable(handle, ctl);
		case I_DISPLAY_ISBUSY: return display_device_isbusy(handle, ctl);
		case I_DISPLAY_GETPALETTE: return SYSFS_SET_RETURN(EINVAL);
	}

	return SYSFS_SET_RETURN(EINVAL);
}

int display_device_read(const devfs_handle_t * handle, devfs_async_t * async){
	return SYSFS_SET_RETURN(ENOTSUP);
}

int display_device_write(const devfs_handle_t * handle, devfs_async_t * async){
	return SYSFS_SET_RETURN(ENOTSUP);
}

int display_device_getinfo(const devfs_handle_t * handle, void * ctl){
	MCU_UNUSED_ARGUMENT(handle);
	display_info_t * info = ctl;
	info->width = DISPLAY_WIDTH;
	info->height = DISPLAY_HEIGHT;
	//info->mem = (void*)DISPLAY_MEMORY;
	//info->scratch_mem = (void*)DISPLAY_SCRATCH;
	//info->size = DISPLAY_MEMORY_SIZE;
	info->cols = DISPLAY_WIDTH;
	info->rows = DISPLAY_HEIGHT;
	info->freq = 30;
	info->bits_per_pixel = 32;
	info->margin_left = 0;
	info->margin_right = 0;
	info->margin_top = 0;
	info->margin_bottom = 0;
	return SYSFS_RETURN_SUCCESS;
}

int display_device_clear(const devfs_handle_t * handle, void * ctl){
	MCU_UNUSED_ARGUMENT(handle);
	MCU_UNUSED_ARGUMENT(ctl);

	u32 color = 0;
	if( ctl != 0 ){
		color = (u32)ctl;
	}
	//previous operation must be complete before refreshing
	HAL_DMA2D_PollForTransfer(&hdma2d, HAL_MAX_DELAY);

	//go to register to memory mode so that screen is filled
	if( hdma2d.Init.Mode != DMA2D_R2M ){
		hdma2d.Init.Mode = DMA2D_R2M;
		if (HAL_DMA2D_Init(&hdma2d) != HAL_OK){
			return SYSFS_SET_RETURN(EIO);
		}
	}

	if( HAL_DMA2D_Start(&hdma2d, color, DISPLAY_MEMORY, DISPLAY_WIDTH, DISPLAY_HEIGHT) != HAL_OK ){
		mcu_debug_printf("failed to start DMA2D %d\n", hdma2d.State);
		return SYSFS_SET_RETURN(EIO);
	}

	mcu_core_invalidate_data_cache_block((void*)DISPLAY_MEMORY, DISPLAY_MEMORY_SIZE);

	HAL_DMA2D_PollForTransfer(&hdma2d, HAL_MAX_DELAY);


	return SYSFS_RETURN_SUCCESS;
}

int display_device_refresh(const devfs_handle_t * handle, void * ctl){

#if 0
	//mcu_debug_printf("refresh display\n");
	u32 * source = (u32*)DISPLAY_MEMORY; //u64 works about twice as fast as u32
	u32 * dest = (u32*)DISPLAY_MEMORY_RAW;

	//for(u32 i=0; i < DISPLAY_WIDTH*DISPLAY_HEIGHT/2; i++){
		//this is way faster than memcpy 15ms vs 88ms with memcpy
		//*dest++ = *source++;
	//}


	//transposing takes about 30ms
	for(u32 i=0; i < DISPLAY_HEIGHT; i++){
		source = (u32*)(DISPLAY_MEMORY + i*4);
		for(u32 j=0; j < DISPLAY_WIDTH; j++){
			*dest++ = *source;
			source+=DISPLAY_HEIGHT;
		}
	}
#else
	//we need to clean the cache (writes values in cache out to memory so DMA reads the right values)
	mcu_core_clean_data_cache_block((void*)DISPLAY_MEMORY, DISPLAY_MEMORY_SIZE);

	//previous operation must be complete before refreshing
	HAL_DMA2D_PollForTransfer(&hdma2d, HAL_MAX_DELAY);

	if( hdma2d.Init.Mode != DMA2D_M2M ){
		hdma2d.Init.Mode = DMA2D_M2M;
		if (HAL_DMA2D_Init(&hdma2d) != HAL_OK){
			return SYSFS_SET_RETURN(EIO);
		}
	}

	//this returns immediately but takes about 8ms to complete
	if( HAL_DMA2D_Start(&hdma2d, DISPLAY_MEMORY, DISPLAY_MEMORY_RAW, DISPLAY_WIDTH, DISPLAY_HEIGHT) != HAL_OK ){
		mcu_debug_printf("failed to start DMA2D %d\n", hdma2d.State);
		return SYSFS_SET_RETURN(EIO);
	}
#if 0
	//executing this will cause the refresh call to block until the operation is complete
	HAL_DMA2D_PollForTransfer(&hdma2d, HAL_MAX_DELAY);
#endif
#endif

	//this needs to trigger the DMA2D to copy from buffer to live memory -- very fast and in sync the LTDC
	return SYSFS_RETURN_SUCCESS;
}

int display_device_init(const devfs_handle_t * handle, void * ctl){

	int result;
	//need to unprotect the external memory
	sys_attr_t attr;

	attr.o_flags = SYS_FLAG_SET_MEMORY_REGION |
			SYS_FLAG_IS_EXTERNAL |
			SYS_FLAG_IS_READ_ALLOWED |
			SYS_FLAG_IS_WRITE_ALLOWED;
	attr.address = DISPLAY_MEMORY;
	attr.size = DISPLAY_MEMORY_SIZE*2;
	attr.region = 0;

	devfs_handle_t sys_handle;
	sys_handle.port = 0;
	sys_handle.config = 0;
	sys_handle.state = 0;

	result = sys_ioctl(&sys_handle, I_SYS_SETATTR, &attr);
	return result;
}

int display_device_enable(const devfs_handle_t * handle, void * ctl){
	return SYSFS_SET_RETURN(ENOTSUP);
}

int display_device_disable(const devfs_handle_t * handle, void * ctl){
	return SYSFS_SET_RETURN(ENOTSUP);
}

int display_device_isbusy(const devfs_handle_t * handle, void * ctl){
	//if transfer is busy this will return true -- zero otherwise
	return HAL_DMA2D_PollForTransfer(&hdma2d, 0) != HAL_OK;
}


