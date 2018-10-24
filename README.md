### key value 根据关键字取值 ###
- **stm32\_key_value** 方便stm32平台芯片内部flash进行键值对存储(f1 f4 L151)；仅支持4字节整型数据(占用8Byte/个UINT32)和字符串数据（至少占用12Byte/个STRINGS）。

### 优点 ###

- **可移植性**：stm32系列芯片非常容易移植，一般只需配置几个宏（有可能需要重写一个简单函数，根据第几扇区获取当前扇区首地址）。

- **存储要求**：ROM:小于3.0KB RAM:24Byte

- **flash操作**：减少内部flash擦除次数，延长flash寿命。

- **存储功能**：扇区写满后，能够保证数据百分百备份成功（即使掉电也能够保证备份百分百成功）。仅仅支持stm32内部flash存储数据。
### 缺点 ###
- 有可能更新某个key的值的时候刚好掉电，那相应key的值有可能丢失；例如：新的key的hash值没写成功，则旧的key的值还可以使用，如果新的key已经写了hash值，value值没写成功就掉电了，那么新的key的值就丢失了，需要程序判断相应key的value值是否正常（当进行key_value存储的时候掉电，存在丢失key的值的风险）。

### 存储原理 ###
- 由key字符串生成一个4字节整型hash值，然后通过hash值找到相应的key的value值（key的hash值和value值都存储在内部flash）。（只有STRINGS的key的value值与key的hash值有可能产生冲突，概率非常小）；当更新某个key的value值的时候，先把新的key的hash和value值写到flash（防止掉电把旧的key_value值丢失），然后再把旧的key的hash和value值擦除( 向flash写0x00(stm32f1或stm32f4系列)或0xff(stm32l系列) ）；UINT32、STRINGS、备份区域（仅当UINT32或STRINGS扇区写满的时候才把当前扇区备份到备份扇区，然后重新覆盖回原来扇区（覆盖有防止断电丢失数据，能够保证覆盖百分百完成）；一个扇区（2KB）最多只能存储255(2048/8 - 1)个不同key的4Byte整型数据）。

### stm32系列芯片移植key_value功能 ###

- **transplant.h 配置相应宏**

	CORTEX_M3表示F1和L151系列

	CORTEX_M4表示F4系列

	\#define STM32L //表示stm32L151系列，因为stm32L系列，stm8S系列，stm8L系列flash属性和F1、F4不一样，因此做特殊处理
	
	\#define SYS false //true带freeRTOS false不带freeRTOS
	
	\#define STRINGS\_HEAD_FLAG 0xef1234ef //default
	
	\#define UINT32\_INIT_FLAG 0x1024 //default
	
	\#define STRINGS\_INIT_FLAG "OK" //default
	
	\#define SECTOR\_NUM 1 //stm32l151系列，一个扇区只有256Byte，1表示仅用一个扇区存储数据
	
	\#define SECTOR\_TOTAL_NUM 8 //stm32f407vet6有8个扇区，因此配置成8，具体可以查询j-flash工具
	
	\#define KEY\_VALUE_SIZE ( 128 * 1024 ) //使用了5/6/7扇区，最小一个扇区是128KB，所以填128*1024;如果选择1/2/3扇区，最小一个扇区是 16KB，那么就填写( 16 * 1024 )
	
	\#define FLASH\_MAX_SIZE ( 512 * 1024 ) //stm32f407vet6芯片内部flash大小为512KB
	
	\#define FLASH\_END\_ADDR ( FLASH\_BASE + FLASH\_MAX_SIZE )//最大的flash地址

- **transplant.c 移植函数**

	移植函数 uint32\_t flash\_sector\_address( int16_t index ) //除了配置transplant.h宏之外，当stm32内部flash扇区大小是不规则分布的时候，需要重写这个函数。举例：stm32f103rct6内部flash的每个扇区都是2KB，因此stm32f103rct6不需要重写这个函数；但stm32f407vet6内部flash分别是16/16/16/16/64/128/128/128，其内部flash扇区为不规则大小；因此stm32f407vet6需要移植这个函数，即根据第几个扇区获取当前扇区的首地址（函数功能）。

### 初始化 ###

	init_key_value( (5), (6), (7) );
	不同key生成的hash值有可能出现一样，因此需要检测所有key生成的hash值是否有冲突。
	检查hash冲突接口：check_hash_conflict( 5, "liang", "zhang", "gan", "hao", "liu" );
### 测试 ###

- 初始化key_value后直接调用测试函数key_value_test测试即可测试:stm32l151c8、stm32f407vet6、stm32f103rct6、stm32f103zet6、stm32f103c8t6、stm32l151rct6芯片; 均稳定运行)

		void key_value_test( void ){
			volatile uint16_t test_mode = 0x00;
			uint32_t i = 0;
			uint32_t j = 0;
			for( i = 0; i < 1111111; i++ ){
			    if( set_key_value( "key_value_test", UINT32, ( uint8_t * )( &i )) ){
			        if( get_key_value( "key_value_test", UINT32, ( uint8_t * )( &j )) && j == i ){
			            KEY_VALUE_INFO( "%d\r\n", j );
			        }else{
			            while( true );
			        }
			    }else{
			        while( true );
			    }
			}
			
			uint32_t test_string = 0;
			uint8_t my_string_test[ 16 ] = "";
			for( uint32_t i = 1111111111; i > 0 ; i-- ){
			    memset( my_string_test, 0, 16 );
			    sprintf( (char *)my_string_test, "%d\r\n", i );
			    if( set_key_value( "my_string_test", STRINGS, (uint8_t *)my_string_test ) ){
			        if( get_key_value( "my_string_test", STRINGS, (uint8_t *)(&test_string) ) ){
			            volatile uint32_t test  = atoi( (char *)test_string );
			            if( test == i ){
			                KEY_VALUE_INFO( "%s", (( uint8_t * )test_string) );
			            }else{
			                volatile bool flag = true;
			                while( flag );
			            }
			        }else{
			            volatile bool flag = true;
			            while( flag );
			        }
			    }else{
			        while( true );
			    }
			}
		}

### 重启次数 ###

	void reboot_times_history_info( void ){
	
		uint32_t reboot_times = 0;
		
		get_key_value( "reboot_times", UINT32, (uint8_t *)(&reboot_times) );
		
		LOG_INFO( "reboot_times_history_info: %d\r\n", reboot_times );
		
		reboot_times ++;
		
		set_key_value( "reboot_times", UINT32, (uint8_t *)(&reboot_times) );
	}

### 获取随机数 ###

	unsigned char math_rand(void){
	
		uint32_t RAND_SEED = 255;
		
		get_key_value( "RAND_SEED", UINT32, (uint8_t *)&RAND_SEED );
		
		RAND_SEED = RAND_SEED * 1103515845 + 3721;
		
		set_key_value( "RAND_SEED", UINT32, (uint8_t *)&RAND_SEED );
		
		return (RAND_SEED/9527) % 32768;
	}
