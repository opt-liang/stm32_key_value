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
	
    taskENTER_CRITICAL();//Ω¯»Î¡ŸΩÁ«¯
    HAL_FLASH_Unlock();
	HAL_FLASHEx_Erase(&f, &PageError);
    HAL_FLASH_Lock();
    taskEXIT_CRITICAL();//ÕÀ≥ˆ¡ŸΩÁ«¯
	
    if( PageError != 0xFFFFFFFF ){
        cycleCount ++;
        if( cycleCount >= 5 ){
            DEBUG_INFO("erase failed\r\n");
            return false;
        }
        osDelay(100);
        goto reerase;
    }
    return true;
}

#if 1
bool flash_write( const uint8_t *ramaddr, uint32_t flashaddr, int16_t size ){
    
    if( ( flashaddr < 0x8000000 || flashaddr > ( 0x8000000 + 256 * 1024 ) ) || size == 0){
        DEBUG_INFO("flash_write: para error\r\n");
        return false;
    }
    
//    if( size != 4 && ((flashaddr - 0x8000000) % 512) != 0 ){
//		DEBUG_INFO("flash_write: ROM address is wrong\r\n");
//        return false;
//    }
    
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
	
    taskENTER_CRITICAL();//Ω¯»Î¡ŸΩÁ«¯
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
	taskEXIT_CRITICAL();//ÕÀ≥ˆ¡ŸΩÁ«¯
    return true;
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

/* ÂêàÂπ∂ÂÜôÂÖ• ******************************************************************************************/
/* ÂÜÖÈÉ®Ê†ºÂºèÔºö0 - 0x10: Ê†áÂøó‰Ωç  ËøôÂêé‰∏∫Êï∞ÊçÆ
 * 
 */
static bool _FlashCopyWrite(uint32_t readdataaddr, uint32_t writesectoraddr, uint16_t len)
{
  uint8_t  temp[16],check[16];
  uint16_t i, sizelen;

  if(AddrSectorOffset(readdataaddr)%4!=0 ) 
  {
    SYS_printf("readdataaddr error:0x%X\r\n", AddrSectorOffset(readdataaddr) );
    return false;
  }
  
  for(i = 0; i<len; i += 16)
  {
		if(flash_read(readdataaddr + i, temp, 16)==false) 
		{
		  SYS_printf("read fail:%u\r\n", i);
		  return false;
		}
		if(len - i>16) sizelen = 16;
		  else sizelen = len - i;
    if(sizelen==0) break;
	  if(flash_write(temp, SectorBaseCount(writesectoraddr) + AddrSectorOffset(readdataaddr) + i, sizelen)==false) 
	  {
			SYS_printf("write fail:%u\r\n", i);
	    return false;
	  }
	  // check 
	  if(flash_read(SectorBaseCount(writesectoraddr) + AddrSectorOffset(readdataaddr) + i, check, sizelen)==false)
	  {
		  SYS_printf("read check:%u\r\n", i);
		  return false;
	  }
	  if(LIB_8BitCompare(temp, check, sizelen)==false) 
	  {
	    SYS_printf("compare fail:%u\r\n", i);
	    _Infor("data", temp, sizelen, _HEX_);
	    _Infor("check", check, sizelen, _HEX_);
	    return false;
	  }
	}
	
	return true;
}

static bool _FlashWriteCheck(uint32_t falshaddr, uint8_t *tdata, uint16_t tdatalen)
{
  uint8_t  temp[16];
  uint16_t i, sizelen;
  
	if(flash_write(tdata, falshaddr, tdatalen)==false) return false;
	// check data
	for(i = 0; i<tdatalen; i += 16)
	{
	  if(tdatalen - i > 16) sizelen = 16;
	    else sizelen = tdatalen - i;
	  if(sizelen==0) break;  
	  if(flash_read(falshaddr + i, temp, sizelen)==false) return false;
	}
	
	return true;
}

bool _FlashWriteAssign(uint32_t flashaddr, uint32_t backupaddr, uint8_t *tdata, uint16_t datalen)
{  
  if(flashaddr>MCU_MAX_ADDR || flashaddr<MCU_MIN_ADDR || flashaddr%4!=0 ) return false;

  backupaddr = SectorBaseCount(backupaddr);
  if( backupaddr==SectorBaseCount(flashaddr) ) return false;

  if( datalen>MCU_SECTOR_SIZE ) datalen = MCU_SECTOR_SIZE;
    else if( datalen%4!=0 )
    {
      datalen = 4 * ( datalen / 4 + 1 );
    }
    
  // erase backup
  SYS_printf("backup erase\r\n");
  if(flash_erase(backupaddr, 1)==false) return false;
  // copy forepart
  SYS_printf("backup copy\r\n");
  if(_FlashCopyWrite(SectorBaseCount(flashaddr), backupaddr, flashaddr - SectorBaseCount(flashaddr))==false) return false;
  // write flashaddr data
  SYS_printf("backup write data\r\n");
  if(_FlashWriteCheck(backupaddr + AddrSectorOffset(flashaddr), tdata, datalen)==false) return false;
  // copy Posterior
  SYS_printf("backup copy\r\n");
  if(_FlashCopyWrite(flashaddr + datalen, backupaddr, MCU_SECTOR_SIZE - (AddrSectorOffset(flashaddr) + datalen))==false) return false;

  // erase data
  SYS_printf("flash erase\r\n");
  if(flash_erase(SectorBaseCount(flashaddr), 1)==false) return false;
  // copy to flashaddr
  SYS_printf("flash copy\r\n");
  if(_FlashCopyWrite(backupaddr, SectorBaseCount(flashaddr), MCU_SECTOR_SIZE)==false) return false;
  // creat complete flag
  SYS_printf("backup write flag\r\n");
  uint8_t flag[EEPROM_DONE_STATE_LEN] = EEPROM_VALID_FLAG;
  if(_FlashWriteCheck(backupaddr, flag, EEPROM_DONE_STATE_LEN)==false) return false;
  SYS_printf("flash write flag\r\n");
  if(_FlashWriteCheck(SectorBaseCount(flashaddr), flag, EEPROM_DONE_STATE_LEN)==false) return false;
  
  return true;
}

#if 1  // test
void FLASH_ReadSectorTest(uint32_t flashaddr)
{
  uint8_t temp[16];
  
  for(uint16_t i = 0; i<MCU_SECTOR_SIZE; i += 16) 
  {
    SYS_printf(">>FLASH[%u]",SectorBaseCount(flashaddr) + i);
    if(flash_read(SectorBaseCount(flashaddr) + i, temp, 16)==false) return;
    _Infor("", temp, 16, _HEX_);
  }
}

#endif

