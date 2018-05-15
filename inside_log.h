#ifndef __INSIDE_LOG__
#define __INSIDE_LOG__

bool UpdateCurrentLogValue( uint32_t flash_addr, int int_variable );
bool GetCurrentLogValue( uint32_t flash_addr, uint32_t* int_variable );
void update_stack_value( void );
void stack_history_use_maximum( void );
void reboot_times_history_info( void );
#endif
