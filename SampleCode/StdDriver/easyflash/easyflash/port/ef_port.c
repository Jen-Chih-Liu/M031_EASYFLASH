/*
 * This file is part of the EasyFlash Library.
 *
 * Copyright (c) 2015-2019, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for each platform.
 * Created on: 2015-01-16
 */
#include <stdio.h>
#include <easyflash.h>
#include <stdarg.h>
#include "NuMicro.h"
/* default environment variables set for user */
static const ef_env default_env_set[] =
{
    {"startup_times", "0"},
    {"pressed_times", "0"},
};

static char log_buf[128];

/**
 * Flash port for hardware initialize.
 *
 * @param default_env default ENV set for user
 * @param default_env_size default ENV size
 *
 * @return result
 */
EfErrCode ef_port_init(ef_env const **default_env, size_t *default_env_size) {
    EfErrCode result = EF_NO_ERR;

    *default_env = default_env_set;
    *default_env_size = sizeof(default_env_set) / sizeof(default_env_set[0]);

    return result;
}

/**
 * Read data from flash.
 * @note This operation's units is word.
 *
 * @param addr flash address
 * @param buf buffer to store read data
 * @param size read bytes size
 *
 * @return result
 */
EfErrCode ef_port_read(uint32_t addr, uint32_t *buf, size_t size) {
    EfErrCode result = EF_NO_ERR;

    /* You can add your code under here. */
    uint8_t *Data = (uint8_t *)buf;

#ifdef dbg
    printf("\r\nef_port_read : 0x%08x, %d\r\n", addr, size);
#endif

    for(size_t i = 0; i < size; i++, addr++, Data++)
    {
        *Data = *(uint8_t *)addr;

#ifdef dbg
        printf("0x%02x, ", *Data);

        if(((i + 1) % 10) == 0)
        {
            printf("\r\n");
        }
#endif
    }

#ifdef dbg
    printf("\r\n");
#endif

    return result;
}

/**
 * Erase data on flash.
 * @note This operation is irreversible.
 * @note This operation's units is different which on many chips.
 *
 * @param addr flash address
 * @param size erase bytes size
 *
 * @return result
 */
EfErrCode ef_port_erase(uint32_t addr, size_t size) {
    EfErrCode result = EF_NO_ERR;

    /* make sure the start address is a multiple of EF_ERASE_MIN_SIZE */
    EF_ASSERT(addr % EF_ERASE_MIN_SIZE == 0);

    /* You can add your code under here. */

    size_t       Number;

    Number = size / FMC_FLASH_PAGE_SIZE;
    if((size % FMC_FLASH_PAGE_SIZE) != 0) Number++;

#ifdef dbg
    printf("\r\nef_port_erase : 0x%08x, %d, %d", addr, size, Number);
#endif

 SYS_UnlockReg();
    /* Enable FMC ISP function */
    FMC_Open();
 FMC_ENABLE_AP_UPDATE();            /* Enable APROM update. */
    for(size_t i = 0; i < Number; i++)
    {
        
        if(FMC_Erase(addr + (FMC_FLASH_PAGE_SIZE * i)) != 0)
        {
            printf("\r\nErase Error!!!");
            result = EF_ERASE_ERR; 
			break;
        }
    }

FMC_DISABLE_AP_UPDATE();           /* Disable APROM update. */
    /* Disable FMC ISP function */
    FMC_Close();

    /* Lock protected registers */
    SYS_LockReg();
	
    return result;
}
/**
 * Write data to flash.
 * @note This operation's units is word.
 * @note This operation must after erase. @see flash_erase.
 *
 * @param addr flash address
 * @param buf the write data buffer
 * @param size write bytes size
 *
 * @return result
 */
EfErrCode ef_port_write(uint32_t addr, const uint32_t *buf, size_t size) {
    EfErrCode result = EF_NO_ERR;

#if 0
    uint32_t ReadAddress = addr;
#endif

    EF_ASSERT((size % 4) == 0);

    /* You can add your code under here. */
 SYS_UnlockReg();
    /* Enable FMC ISP function */
    FMC_Open();
 FMC_ENABLE_AP_UPDATE();            /* Enable APROM update. */

#if 0
    printf("\r\nef_port_write(0x%08x, buf, %d)\r\n", addr, size);

    uint8_t *Data = (uint8_t *)buf;

    for(size_t i = 0; i < size; i++)
    {
        printf("0x%02x, ", *Data++);

        if(((i + 1) % 10) == 0)
        {
            printf("\r\n");
        }
    }

    printf("\r\n");
#endif

    for(size_t i = 0; i < size; i+=4, buf++, addr+=4)
    {       
        FMC_Write(addr, *buf); 

        uint32_t Data = *(uint32_t *)addr;

        if(Data != *buf)
        {
            printf("\r\nWrite Error!!!");
            result = EF_WRITE_ERR; 
			break;
        }
    }

     FMC_DISABLE_AP_UPDATE();           /* Disable APROM update. */
    /* Disable FMC ISP function */
    FMC_Close();

    /* Lock protected registers */
    SYS_LockReg();

#if 0
    uint32_t Buffer[100];
    ef_port_read(ReadAddress, Buffer, size);
#endif

    return result;
}

/**
 * lock the ENV ram cache
 */
void ef_port_env_lock(void) {
    
    /* You can add your code under here. */
    __disable_irq();
}

/**
 * unlock the ENV ram cache
 */
void ef_port_env_unlock(void) {
    
    /* You can add your code under here. */
    __enable_irq();
}


/**
 * This function is print flash debug info.
 *
 * @param file the file which has call this function
 * @param line the line number which has call this function
 * @param format output format
 * @param ... args
 *
 */
void ef_log_debug(const char *file, const long line, const char *format, ...) {

#ifdef PRINT_DEBUG

    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);

    /* You can add your code under here. */
    ef_print("\r\n[Debug](%s:%ld) ", file, line);
    vsprintf(log_buf, format, args);
    ef_print("%s", log_buf);
    printf("\r\n");

    va_end(args);

#endif

}

/**
 * This function is print flash routine info.
 *
 * @param format output format
 * @param ... args
 */
void ef_log_info(const char *format, ...) {
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);

    /* You can add your code under here. */
    ef_print("\r\n[LogInfo]");
    /* must use vprintf to print */
    vsprintf(log_buf, format, args);
    ef_print("%s", log_buf);
    printf("\r\n");

    va_end(args);
}
/**
 * This function is print flash non-package info.
 *
 * @param format output format
 * @param ... args
 */
void ef_print(const char *format, ...) {
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);

    /* You can add your code under here. */
    vsprintf(log_buf, format, args);
    printf("%s", log_buf);

    va_end(args);
}
