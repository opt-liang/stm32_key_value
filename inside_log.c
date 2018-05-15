#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "insideflash.h"
#include "flashaddr.h"
#include "cmsis_os.h"
#include "key_value.h"
#include "commonInterface.h"

#define LOG_DEBUG 1

#if LOG_DEBUG

#define LOG_INFO( fmt, args... ) 	printf( fmt, ##args )//LOG_INFO(fmt, ##args)
#else
#define LOG_INFO(debug, fmt, args... )
#endif

void stack_history_use_maximum( void ){

    extern osThreadId MqttTaskHandle;
    extern osThreadId checkLteTaskHandle;

    uint32_t Mqtt_STACK_SIZE = 0;
    uint32_t freeRTOS_MINI_STACK_SIZE ;
    uint32_t DATA_TEMP = 0;

    get_key_value( "Mqtt_TASK", UINT32, (uint8_t *)(&Mqtt_STACK_SIZE) );
    if( Mqtt_STACK_SIZE == 0x00 ){
        
        DATA_TEMP = uxTaskGetStackHighWaterMark( MqttTaskHandle );
        set_key_value( "Mqtt_TASK", UINT32, (uint8_t *)(&DATA_TEMP) );
        get_key_value( "Mqtt_TASK", UINT32, (uint8_t *)(&Mqtt_STACK_SIZE) );
    }
    
    get_key_value( "freeRTOS_MINI_STACK_SIZE", UINT32, (uint8_t *)(&freeRTOS_MINI_STACK_SIZE) );
    if( freeRTOS_MINI_STACK_SIZE == 0x00 ){
        freeRTOS_MINI_STACK_SIZE = xPortGetMinimumEverFreeHeapSize();
        set_key_value( "freeRTOS_MINI_STACK_SIZE", UINT32, (uint8_t *)(&freeRTOS_MINI_STACK_SIZE) );
    }
    
    if( !GetNetworkMode() ){
       uint32_t LTE_TASK_STACK ;
       get_key_value( "LTE_TASK_STACK", UINT32, (uint8_t *)(&LTE_TASK_STACK) );
       if( LTE_TASK_STACK == 0x00 ){
           LTE_TASK_STACK = uxTaskGetStackHighWaterMark( checkLteTaskHandle ) ;
           set_key_value( "LTE_TASK_STACK", UINT32, (uint8_t *)(&LTE_TASK_STACK) );
           LOG_INFO( "LTE_TASK_STACK stack history use maximum---- %d\r\n", 512 - LTE_TASK_STACK );
       }
    }
    
    LOG_INFO( "Mqtt_TASK stack history use maximum---- %d\r\n", 512 - Mqtt_STACK_SIZE );
    LOG_INFO( "FREERTOS_MINI_STACK_SIZE stack history---- %d\r\n", freeRTOS_MINI_STACK_SIZE );
}

void update_stack_value( void ){
    
    extern osThreadId MqttTaskHandle;
    extern osThreadId checkLteTaskHandle;
    
    uint32_t Mqtt_STACK_SIZE = 0;
    uint32_t DATA_TEMP       = 0;
    uint32_t freeRTOS_MINI_STACK_SIZE ;
    
    
    get_key_value( "Mqtt_TASK", UINT32, (uint8_t *)(&Mqtt_STACK_SIZE) );
    get_key_value( "freeRTOS_MINI_STACK_SIZE", UINT32, (uint8_t *)(&freeRTOS_MINI_STACK_SIZE) );
    
    DATA_TEMP = uxTaskGetStackHighWaterMark( MqttTaskHandle );
    
    if( DATA_TEMP < Mqtt_STACK_SIZE ){
        set_key_value( "Mqtt_TASK", UINT32, (uint8_t *)(&DATA_TEMP) );
        LOG_INFO( "MqttTaskHandle TASK STACK %u\r\n", 512 - DATA_TEMP  );
    }
    
    DATA_TEMP = xPortGetMinimumEverFreeHeapSize();
    if( freeRTOS_MINI_STACK_SIZE > DATA_TEMP ){
        set_key_value( "freeRTOS_MINI_STACK_SIZE", UINT32, (uint8_t *)(&DATA_TEMP) );
        LOG_INFO( "FREERTOS_MINI_STACK_SIZE STACK %u\r\n", DATA_TEMP  );
    }
    
    if( !GetNetworkMode() ){
       uint32_t LTE_TASK_STACK ;
       get_key_value( "LTE_TASK_STACK", UINT32, (uint8_t *)(&LTE_TASK_STACK) );
       DATA_TEMP = uxTaskGetStackHighWaterMark( checkLteTaskHandle ) ;
       if( DATA_TEMP < LTE_TASK_STACK ){
           set_key_value( "LTE_TASK_STACK", UINT32, (uint8_t *)(&DATA_TEMP) );
           LOG_INFO( "LTE_TASK_STACK stack history use maximum---- %d\r\n", 512 - DATA_TEMP );
       }
    }
}

void reboot_times_history_info( void ){
    
    uint32_t reboot_times = 0;
    
    get_key_value( "reboot_times", UINT32, (uint8_t *)(&reboot_times) );
    
    if( reboot_times == 0 ){
        reboot_times = 1;
        set_key_value( "reboot_times", UINT32, (uint8_t *)(&reboot_times) );
    }else{
        reboot_times++;
        set_key_value( "reboot_times", UINT32, (uint8_t *)(&reboot_times) );
    }
    
    LOG_INFO( "reboot_times_history_info: %d\r\n", reboot_times == 0 ? 1 : reboot_times );
}

