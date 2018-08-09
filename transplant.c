#include "transplant.h"


uint32_t flash_sector_address( int16_t index ){
    
    if( index > SECTOR_TOTAL_NUM || index < 1 ){
        printf( "Fan area index error\r\n" );
        while( true );
    }
    
    uint32_t realaddr = FLASH_BASE;
    
    #if defined CORTEX_M3
        realaddr += ( index ) * KEY_VALUE_SIZE;
    #else
        //stm32f407ve       16 16 16 16 64 128 128 128
        if( index > 0 ){
            realaddr += ( index > 3 ? 4 : index ) * 16 * 1024;
        }
        if( index > 4 ){
            realaddr += 1 * 64 * 1024;
        }
        if( index > 5 ){
            realaddr += ( index - 5 ) * 128 * 1024;
        }
    #endif
    
    return realaddr;
}

bool flash_legal_sector_address( int32_t flashaddr ){
    for( uint16_t i = 1; i < SECTOR_TOTAL_NUM; i++ ){
        if( flashaddr == flash_sector_address( i ) ){
            return true;
        }
    }
    return false;
}

int16_t flash_sector_index( uint32_t flashaddr ){
    for( int16_t i = 1; i < SECTOR_TOTAL_NUM; i++ ){
        if( flashaddr == flash_sector_address( i ) ){
            return i;
        }
    }
    return -1;
}





