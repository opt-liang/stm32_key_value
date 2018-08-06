key value 根据关键字取值

#stm32_key_value      stm32 f1 f4 L151系列键值对存储；支持4字节整型数据，字符串类型。仅仅支持stm32内部flash存储数据。

友情链接:支持外部flash存储的另一个开源项目https://github.com/armink/EasyFlash.git

可能产生哈希冲突，需要检测，检查接口  check_hash_conflict( 5, "liang", "zhang", "gan", "hao", "liu" );

配置:


测试过:stm32l151c8、stm32f407vet6、stm32f103rct6、stm32f103zet6、stm32f103c8t6,运行非常稳定

