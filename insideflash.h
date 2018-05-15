#ifndef __INSID__FLASH__H__
#define __INSID__FLASH__H__

#include "configure.h"

#define  MCU_FLASH_OFFSET                   0x08000000
#define  MCU_SECTOR_SIZE                    0x800
#define  SectorCount(exp)                   (MCU_FLASH_OFFSET + exp*MCU_SECTOR_SIZE)  // 2K
#define  SectorBaseCount(addr)              ((addr/MCU_SECTOR_SIZE)*MCU_SECTOR_SIZE)
#define  AddrSectorOffset(addr)             (addr - SectorBaseCount(addr))

#define  FlashAddr4MultipleBaseCount(addr)  (((addr - MCU_FLASH_OFFSET)/4)*4 + MCU_FLASH_OFFSET)

#if (defined(EEPROM_SAVE_DATA_ADDR) || defined(EEPROM_BACKUP_DATA_ADDR))
 #if defined(_MCU_MODEL_STM32F103RCT6_)
  #if (EEPROM_DATA_MAX_SECTOR_NUM>128 || EEPROM_DATA_MAX_SECTOR_NUM>128)
   #error "sector number out of range!"
  #endif
  
  #define MCU_MIN_ADDR            SectorCount(EEPROM_DATA_MIN_SECTOR_NUM) //120*2048 = MCU_FLASH_OFFSET + 3C000
  #define MCU_MAX_ADDR            SectorCount(EEPROM_DATA_MAX_SECTOR_NUM) //128*2048 = MCU_FLASH_OFFSET + 40000
 #endif
 #if (EEPROM_SAVE_DATA_ADDR>=MCU_MAX_ADDR || MCU_MAX_ADDR<MCE_MIN_ADDR || EEPROM_BACKUP_DATA_ADDR>=MCU_MAX_ADDR || MCU_MAX_ADDR<EEPROM_BACKUP_DATA_ADDR)
  #error "EEPROM_SAVE_DATA_ADDR or EEPROM_BACKUP_DATA_ADDR size is error!"
 #endif
#endif

typedef enum
{
  _SPLICE_FRONT_,
  _SPLICE_END_,
}SpliceWayEnum;

bool flash_write( const uint8_t *ramaddr, uint32_t flashaddr, int16_t size );
bool flash_read( int32_t flashAddr , uint8_t *ramBuffer , uint16_t bytesLen );
bool flash_erase( int32_t flashaddr, uint32_t page );
bool _Flash_nByteWrite( const uint8_t *ramaddr, uint32_t flashaddr, int16_t size );
bool _FlashWriteAssign(uint32_t flashaddr, uint32_t backupaddr, uint8_t *tdata, uint16_t datalen);

uint32_t Splice4Byte(uint32_t tdata1, uint32_t tdata2, uint8_t tdata1emptybyte);

void FLASH_ReadSectorTest(uint32_t flashaddr);

#endif

