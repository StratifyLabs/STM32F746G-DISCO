#ifndef DISPLAY_DEVICE_H
#define DISPLAY_DEVICE_H


#include <sos/fs/devfs.h>
#include <sos/dev/display.h>

#define DISPLAY_WIDTH 480
#define DISPLAY_HEIGHT 272
#define DISPLAY_PIXEL_BYTE_SIZE 4
#define DISPLAY_MEMORY 0xC0000000
#define DISPLAY_MEMORY_SIZE (DISPLAY_WIDTH*DISPLAY_HEIGHT*DISPLAY_PIXEL_BYTE_SIZE)
#define DISPLAY_SCRATCH (DISPLAY_MEMORY + DISPLAY_MEMORY_SIZE)
#define DISPLAY_MEMORY_RAW (DISPLAY_SCRATCH + DISPLAY_MEMORY_SIZE)
#define DISPLAY_ROW_SIZE (DISPLAY_WIDTH*4)

int display_device_open(const devfs_handle_t * handle);
int display_device_close(const devfs_handle_t * handle);
int display_device_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int display_device_read(const devfs_handle_t * handle, devfs_async_t * async);
int display_device_write(const devfs_handle_t * handle, devfs_async_t * async);


#endif // DISPLAY_DEVICE_H
