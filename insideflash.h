#ifndef __INSID__FLASH__H__
#define __INSID__FLASH__H__
#include <stdint.h>
#include <stdbool.h>

#define M3      //M4

#if defined M3
//    #define _STM32L_
    #if defined _STM32L_
        #include "stm32l1xx_hal.h"
    #else
        #include "stm32f1xx_hal.h"
    #endif
#else
    #include "stm32f4xx_hal.h"
#endif


#if defined _STM32L_
    #define BACKUP_FLAG_CLEAR_FLAG  0x0000ffff
    #define ERASURE_STATE           0x00000000
    #define FILL_STATE              ~ERASURE_STATE
#else
    #define BACKUP_FLAG_CLEAR_FLAG  0x00000000
    #define ERASURE_STATE           0xffffffff
    #define FILL_STATE              ~ERASURE_STATE
#endif


#define SECTOR_NUM              128
#define SECTOR_SIZE_MIN         ( 2 * 1024 )

#define FLASH_MAX_SIZE          ( 256 * 1024 )
#define FLASH_END_ADDR          ( FLASH_BASE + FLASH_MAX_SIZE )

uint32_t flash_sector_address( int16_t index );
#define  ADDRESS_MAPPING(X)     flash_sector_address( X )

bool flash_write( const uint8_t *ramaddr, uint32_t flashaddr, int32_t size );
bool flash_read( int32_t flashAddr , uint8_t *ramBuffer , uint16_t bytesLen );
bool flash_erase( int32_t flashaddr, uint32_t page );

#endif

