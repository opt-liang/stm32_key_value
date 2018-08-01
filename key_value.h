#ifndef __KEY_VALUE__
#define __KEY_VALUE__
#include <stdint.h>
#include <stdbool.h>
#include "insideflash.h"
enum TYPE{
    UINT32   ,
    STRINGS  ,
};
void key_value_test( void );
void check_hash_conflict( int count,...);
bool get_key_value( char *key, enum TYPE type , uint8_t *value );
bool set_key_value( char *key, enum TYPE type, uint8_t *value );
void init_key_value( uint32_t key_value_int32, uint32_t key_value_string, uint32_t key_value_backup );
#endif

