#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "key_value.h"
#include "insideflash.h"
#include "flashaddr.h"
#include "cmsis_os.h"

#define KEY_DEBUG 1

#if KEY_DEBUG

#define KEY_VALUE_INFO( fmt, args... ) 	printf( fmt, ##args )//KEY_VALUE_INFO(fmt, ##args)
#else
#define KEY_VALUE_INFO( fmt, args... )
#endif

/************************Simple transplantable flash operation**********************/

#define HASH_MAX_SIZE   ( 2 * 1024 )

enum BACKUP_FLAG{
    BACKUP_FLAG_CLEAR   = 0x00000000,
    UINT32_FLAG         = 0x00009600,
    STRINGS_FLAG        = 0x00006900,
};

SemaphoreHandle_t key_value_SemaphoreHandle;

//Only two kinds of key-value are supported;    namely int or string type
uint32_t KEY_VALUE_INT32 = 0;
uint32_t KEY_VALUE_STRINGS = 0;
uint32_t KEY_VALUE_BACKUP = 0;

uint32_t* __find_key( uint32_t key, enum TYPE type ){
    
    //find string value
    if( type == UINT32 ){
        uint32_t *address = (uint32_t *)KEY_VALUE_INT32;
        for( uint16_t i = 0; i < ( HASH_MAX_SIZE / 8 - 1 ); i ++ ){
            if( key == *( address + i * 2 ) ){
                return ( address + i * 2 );
            }
        }
    }else if( type == STRINGS ){
        uint32_t *address = (uint32_t *)KEY_VALUE_STRINGS;
        for( uint16_t i = 0; i < ( HASH_MAX_SIZE / 4 - 2 ); i ++ ){
            if( 0xefffffff == *( address + i ) ){
                if( key == *( address + i + 1 ) ){
                    return ( address + i + 1 );
                }
            }else if( key == *( address + i ) ){
                return ( address + i );
            }
        }
    }
    return NULL;
}

bool backup_flag( enum TYPE type, bool stat ){
    
    uint32_t backup_success = 0;
    if( stat ){
        if( type == UINT32 ){
            backup_success = UINT32_FLAG;
        }else if( type == STRINGS ){
            backup_success = STRINGS_FLAG;
        }
    }else{
        backup_success = BACKUP_FLAG_CLEAR;
    }
    stat = false;
    if( flash_write( (const uint8_t *)( &backup_success ), (uint32_t)( KEY_VALUE_BACKUP + HASH_MAX_SIZE - 4 ), 4 ) ){
        stat = true;
    }
    return stat;
}

uint8_t get_backup_flag( enum TYPE type ){
    bool stat = false;
    if( type == UINT32 ){
        if( *(uint32_t *)( KEY_VALUE_BACKUP + HASH_MAX_SIZE - 4 ) == UINT32_FLAG ){
            stat = true;
        }
    }else if( type == STRINGS ){
        if( *(uint32_t *)( KEY_VALUE_BACKUP + HASH_MAX_SIZE - 4 ) == STRINGS_FLAG ){
            stat = true;
        }
    }
    return stat;
}

bool move_key_value_back( enum TYPE type ){
    bool stat = false;
    if( type == UINT32 ){
        
        if( flash_erase( KEY_VALUE_INT32, 1 ) == false ){
            return false;
        }
        flash_write( (const uint8_t *)( KEY_VALUE_BACKUP ), (uint32_t)( KEY_VALUE_INT32 ), HASH_MAX_SIZE - 8 );
        if( memcmp( (const uint8_t *)(KEY_VALUE_INT32), (const uint8_t *)( KEY_VALUE_BACKUP ), HASH_MAX_SIZE - 8 ) == 0 ){
            stat = true;
        }
    }else if( type == STRINGS ){
        
        if( flash_erase( KEY_VALUE_STRINGS, 1 ) == false ){
            return false;
        }

        flash_write( (const uint8_t *)( KEY_VALUE_BACKUP ), (uint32_t)( KEY_VALUE_STRINGS ), HASH_MAX_SIZE - 8 );
        if( memcmp( (const uint8_t *)( KEY_VALUE_STRINGS ), (const uint8_t *)( KEY_VALUE_BACKUP ), HASH_MAX_SIZE - 8 ) == 0 ){
            stat = true;
        }
    }
    if( stat ){
        backup_flag( type, false );
        flash_erase( KEY_VALUE_BACKUP, 1 );
    }
    return stat;
}

bool move_key_value( enum TYPE type ){
    
    if( flash_erase( KEY_VALUE_BACKUP, 1 ) == false ){
        return false;
    }
    
    if( type == UINT32 ){
        
        uint32_t *address = (uint32_t *)KEY_VALUE_INT32;
        for( uint16_t i = 0, j = 0; i < ( HASH_MAX_SIZE / 8 ); i ++ ){
			if( 0x00000000 != *( address + i * 2 ) ){
                if( 0xffffffff == *( address + i * 2 ) ){
                    break;
                }
				if( flash_write( (const uint8_t *)(address + i * 2), (uint32_t)( KEY_VALUE_BACKUP + j * 2 * 4 ), 8 ) ){
                    j++;
                }else{
                    return false;
                }
			}
        }
        
        backup_flag( type, true );

        if( move_key_value_back( type ) ){
            return true;
        }
        
		return false;
        
    }else if( type == STRINGS ){
        
        uint32_t *address = (uint32_t *)KEY_VALUE_STRINGS;
        
        for( uint16_t i = 0, j = 0; i < ( HASH_MAX_SIZE / 4 ); i ++ ){
            if( 0x00000000 != *( address + i ) ){
                if(  0xffffffff == *( address + i ) ){
                    break;
                }
                if( flash_write( (const uint8_t *)(address + i ), (uint32_t)( KEY_VALUE_BACKUP + j * 4 ), 4 ) ){
                    j++;
                }else{
                    return false;
                }
            }
        }
        
        backup_flag( type, true );
        
        if( move_key_value_back( type ) ){
            return true;
        }
        return false;
    }else{
        return false;
    }
}

void init_key_value( uint32_t key_value_int32, uint32_t key_value_string, uint32_t key_value_backup ){
    
    KEY_VALUE_INT32 = key_value_int32;                //ADDRESS_MAPPING(17)
    KEY_VALUE_STRINGS = key_value_string;
	KEY_VALUE_BACKUP = key_value_backup;
    
    key_value_SemaphoreHandle = xSemaphoreCreateBinary();//signal
    xSemaphoreGive( key_value_SemaphoreHandle );
    if( key_value_SemaphoreHandle == NULL ){
        KEY_VALUE_INFO("init_key_value key_value_SemaphoreHandle failed\r\n");
        while( true );
    }
    
    if( KEY_VALUE_INT32 != 0 ){                         //Determine whether the address is valid or not, within the chip address range. 
        if( get_backup_flag( UINT32 ) ){
            move_key_value_back( UINT32 );
        }
    }
    
    if( KEY_VALUE_STRINGS != 0 ){
        if( get_backup_flag( STRINGS ) ){
            move_key_value_back( STRINGS );
        }
    }
    
    if( KEY_VALUE_BACKUP != 0 ){
        flash_erase( KEY_VALUE_BACKUP, 1 );
    }
}

/************************Simple transplantable key value****************************/

//Conflict situation

int aphash( char* str ){
      
    int hash = 0;

    uint16_t len = strlen( str );
    if( len > 64 ){
      KEY_VALUE_INFO( "aphash String over 64 bytes\r\n");
    }
    for (int i = 0; i < len && len <= 64; i++) {
     if ((i & 1) == 0) {
        hash ^= (hash << 7) ^ ( str[i] ) ^ (hash >> 3);
     } else {
        hash ^= ~((hash << 11) ^ ( str[i] ) ^ (hash >> 5));
     }
    }

    return hash & 0x7FFFFFFF;
}

//Hash conflict check, function check all string key

bool check_repetition( uint32_t *array, uint8_t count ){

    for( uint8_t i = 0; i < count && count < 255; i ++ ){
        for( uint8_t j = i+1; j < count; j ++ ){
            if( array[i] == array[j] ){
                    return true;
            }
        }
    }
    return false;
}

// check_hash_conflict( 5, "liang", "zhang", "gan", "hao", "liu" );
void check_hash_conflict( int count,...){
    if( count >= 200 ){
        KEY_VALUE_INFO( "check_hash_conflict--The number is greater than 255\r\n");
        return;
    }
    
    va_list ap;
    char *s;
    uint32_t *array = NULL;

    array = (uint32_t *)pvPortMalloc( count * sizeof(uint32_t) );
    
    if( array == NULL ){
        KEY_VALUE_INFO( "Allocated memory failure\r\n");
        return;
    }
    
    va_start(ap, count);
    
    for( uint8_t i = 0; i < count ; i ++ ){
        s = va_arg( ap, char*);
        array[i] = aphash( s );
        KEY_VALUE_INFO( "string %s\n",s);
    }
    
    va_end(ap);
    
    if ( check_repetition( array, count) ){
        KEY_VALUE_INFO( "Duplication of data\r\n");
    }else{
        KEY_VALUE_INFO( "No Duplication of data\r\n");
    }
    
    vPortFree( array );
}

/**********************************************************************************************/
//UINT32        VALUE
//STRINGS       POINTER
bool get_key_value( char *key, enum TYPE type , uint8_t *value ){
    
    bool stat = false;
    static int16_t local_flag = 0;

    if( local_flag == 0 ){
        xSemaphoreTake( key_value_SemaphoreHandle, portMAX_DELAY );
        local_flag ++;
    }
    
    int hash = aphash( key );//Possible strings generate hash conflicts

    //find string value
    *((uint32_t *)value) = 0x00;
    uint32_t* key_address = __find_key( hash, type );
    
    if( key_address ){
        if( type == UINT32 ){
            *((uint32_t *)value) = *( key_address + 1 );
        }else if( type == STRINGS ){
            *((uint32_t *)value) = ( uint32_t )( key_address + 1 );
        }
        stat = true;
    }
    
    if( --local_flag == 0 ){
        xSemaphoreGive( key_value_SemaphoreHandle );
    }
    
    return stat;
}

bool set_key_value( char *key, enum TYPE type, uint8_t *value ){
    
    xSemaphoreTake( key_value_SemaphoreHandle, portMAX_DELAY);
    bool stat = false;
    int hash = aphash( key );//Possible strings generate hash conflicts

    //find string value
    if( type == UINT32 ){
        
        uint32_t* key_address = __find_key( hash, type );
        
        if( key_address ){
            uint32_t variable = 0;
            flash_write( (const uint8_t *)(&variable), (uint32_t)( key_address ), 4);           //flash write 0
            flash_write( (const uint8_t *)(&variable), (uint32_t)( key_address + 1 ), 4);       //+4
        }
        
        uint32_t* addr = NULL;
        
        int32_rewrite:
        
        addr = __find_key( 0xffffffff, type );
        
        if( addr ){
            
            //write real value
			flash_write( (const uint8_t *)(&hash), (uint32_t)( addr ), 4);
			flash_write( (const uint8_t *)(value), (uint32_t)( addr + 1 ), 4);
            
			//compare real value
			if( *(addr) == hash && *((addr + 1) ) == *((uint32_t *)value) ){
                stat = true;
			}
        }else{

            //write to backup
			if( move_key_value( type ) == false ){
                KEY_VALUE_INFO("UINT32 write to backup failed\r\n");
				goto exe;
			}
			
            static uint8_t cycleCount = 0;
            if( cycleCount ++ > 5 ){
                cycleCount = 0;
                KEY_VALUE_INFO( "UINT32 set_key_value backup failed\r\n" );
                goto exe;
            }
            goto int32_rewrite;
        }
    }else if( type == STRINGS ){
        
        volatile uint32_t* key_address = __find_key( hash, type );
        
        if( key_address ){
            uint16_t i = 1;
            uint32_t variable = 0;
            flash_write( (const uint8_t *)(&variable), (uint32_t)( key_address - 1 ), 4);
            flash_write( (const uint8_t *)(&variable), (uint32_t)( key_address ), 4);
            
            do{
                if( (*( key_address + i ) & 0xff000000) == 0x00000000 ){
                    flash_write( (const uint8_t *)(&variable), (uint32_t)( key_address + i ), 4);
                    break;
                }
                flash_write( (const uint8_t *)(&variable), (uint32_t)( key_address + i++ ), 4);
            }while( ( uint32_t )( key_address + i ) < ( KEY_VALUE_STRINGS + HASH_MAX_SIZE ) );
            
            if( ( uint32_t )( key_address + i ) >= ( KEY_VALUE_STRINGS + HASH_MAX_SIZE ) ){
                KEY_VALUE_INFO( "set_key_value Abnormality error\r\n" );
                goto exe;
            }
        }
        
        uint32_t* addr = NULL;
        uint16_t len  = strlen( ( char *)value );
        if( len > 256 ){
            KEY_VALUE_INFO( "key_value String over 256 bytes\r\n" );
            return false;
        }
        strings_rewrite:
        addr = __find_key( 0xffffffff, type );
        if( addr && ( uint32_t )( addr + 2 + len / 4 + 1 ) < ( KEY_VALUE_STRINGS + HASH_MAX_SIZE ) ){
            uint32_t variable = 0;
            if( len % 4 == 0x00 ){
                flash_write( ( uint8_t * )( &variable ), ( uint32_t )( addr + 2 + len / 4 + 1 ), 4 );
            }
            variable = 0xefffffff;
            flash_write( (uint8_t *)(&variable), (uint32_t)( addr ), 4);
            flash_write( (uint8_t *)(&hash), (uint32_t)( addr + 1 ), 4);
            flash_write( value, (uint32_t)( addr + 2 ), len );

            if( memcmp( value, addr + 2, len ) == 0x00 ){
                stat = true;
            }
        }else{
            
            //write to backup
			if( move_key_value( type ) == false ){
                KEY_VALUE_INFO("STRINGS write to backup failed\r\n");
				goto exe;
			}
            
            static uint8_t cycleCount = 0;
            if( cycleCount ++ > 5 ){
                cycleCount = 0;
                KEY_VALUE_INFO( "STRINGS set_key_value backup failed\r\n" );
                goto exe;
            }
            goto strings_rewrite;
        }
    }
    
    exe:
    xSemaphoreGive( key_value_SemaphoreHandle );
    return stat;
}









