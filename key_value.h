#ifndef __KEY_VALUE__
#define __KEY_VALUE__
enum TYPE{
    UINT32   ,
    STRINGS  ,
};

bool get_key_value( char *key, enum TYPE type , uint8_t *value );
bool set_key_value( char *key, enum TYPE type, uint8_t *value );
void init_key_value( uint32_t key_value_int32, uint32_t key_value_string, uint32_t key_value_backup );
#endif

