#include "commonInterface.h"
#include "insideflash.h"
#include "sharelib.h"
#include "putinfor.h"

/*******************************flash operation******************************************************/

bool flash_erase( int32_t flashaddr, uint32_t page ){
    
    if( (( flashaddr < 0x8000000 || flashaddr > ( 0x8000000 + 256 * 1024 ) ) && ( flashaddr - 0x8000000 ) % 2048 ) || page == 0){
        DEBUG_INFO("flash_erase: erase para error\r\n");
        return false;
    }
    
    uint8_t cycleCount = 0;
	
//	uint16_t shiftflash = ( (flashaddr - 0x8000000) / 1024 % 2 ) * 1024;
    
    FLASH_EraseInitTypeDef f;
    
    f.TypeErase = FLASH_TYPEERASE_PAGES;
    
    f.PageAddress = flashaddr;//flashaddr - shiftflash;
    
    f.NbPages = page;
    
    uint32_t PageError = 0;
	
reerase:
	
    taskENTER_CRITICAL();//½øÈëÁÙ½çÇø
    HAL_FLASH_Unlock();
	HAL_FLASHEx_Erase(&f, &PageError);
    HAL_FLASH_Lock();
    taskEXIT_CRITICAL();//ÍË³öÁÙ½çÇø
	
    if( PageError != 0xFFFFFFFF ){
        cycleCount ++;
        if( cycleCount >= 5 ){
            DEBUG_INFO("erase failed\r\n");
            return false;
        }
        osDelay(100);
        goto reerase;
    }

    uint8_t *eraseaddr = (uint8_t *)flashaddr;
    for( uint32_t i = 0; i < page * 2048 ; i ++ ){
        if( eraseaddr[ i ] != 0xff ){
            return false;
        }
    }

    return true;
}

#if 1
bool flash_write( const uint8_t *ramaddr, uint32_t flashaddr, int16_t size ){
    
    if( ( flashaddr < 0x8000000 || flashaddr > ( 0x8000000 + 256 * 1024 ) ) || size == 0){
        DEBUG_INFO("flash_write: para error\r\n");
        return false;
    }
    
    if( ( (flashaddr - 0x8000000) % 4) != 0 ){
		DEBUG_INFO("flash_write: Writing a int address is wrong\r\n");
        return false;
    }
	
	if( size > 2048 ){
		DEBUG_INFO("The amount of data is too large\r\n");
		return false;
	}
	
    uint32_t currentflashAddr   = flashaddr;
    const uint8_t  *currentram  = ramaddr;
    uint8_t  Remainder          = size % 4;
    uint16_t Multiple           = size / 4;
	
    taskENTER_CRITICAL();//½øÈëÁÙ½çÇø
    HAL_FLASH_Unlock();
    for( int16_t i = 0; i < Multiple ; i ++  ){
        
        HAL_FLASH_Program( FLASH_TYPEPROGRAM_WORD, currentflashAddr, *((uint32_t *)currentram) );
        currentflashAddr += 4;
        currentram += 4;
    }
    
    if( Remainder ){
        uint8_t data[4] ;
		memset( data, 0x00, 4);//change
        for( uint8_t i = 0; i < Remainder; i++ ){
            data[i] = currentram[i];
        }
        HAL_FLASH_Program( FLASH_TYPEPROGRAM_WORD, currentflashAddr,  *((uint32_t *)data) );
    }
    
    HAL_FLASH_Lock();
	taskEXIT_CRITICAL();//ÍË³öÁÙ½çÇø
    
    if( memcmp( ramaddr, (uint8_t *)flashaddr, size) == 0 ){
        return true;
    }else{
        DEBUG_INFO("inside flash write failure\r\n");
        return false;
    }
}
#endif

bool flash_read( int32_t flashaddr , uint8_t *ramBuffer , uint16_t bytesLen ){
    if( (flashaddr < 0x8000000 || flashaddr > ( 0x8000000 + 256 * 1024 )) ||  bytesLen == 0){
        DEBUG_INFO("flash_read :ROM address is wrong\r\n");
        return false;
    }
	while( bytesLen -- ){
	 *( ramBuffer ++ ) = *(( uint8_t* ) flashaddr ++ );
	}
    return true;
}
