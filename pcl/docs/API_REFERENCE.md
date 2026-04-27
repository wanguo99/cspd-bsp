# XConfig API 参考手册

## 初始化和注册

### HW_Config_Init
初始化配置库。
```c
int32_t HW_Config_Init(void);
```

### HW_Config_RegisterAll
注册所有配置。
```c
int32_t HW_Config_RegisterAll(void);
```

### HW_Config_SelectDefault
选择默认配置。
```c
const xconfig_board_config_t* HW_Config_SelectDefault(void);
```

### HW_Config_GetBoard
获取当前板级配置。
```c
const xconfig_board_config_t* HW_Config_GetBoard(void);
```

### HW_Config_Print
打印配置信息。
```c
void HW_Config_Print(const xconfig_board_config_t *board);
```

## 外设配置查询

### XCONFIG_HW_FindMCU
查找MCU外设配置。
```c
const xconfig_mcu_cfg_t* XCONFIG_HW_FindMCU(const xconfig_board_config_t *board,
                                            const str_t *name);
```

### XCONFIG_HW_FindBMC
查找BMC外设配置。
```c
const xconfig_bmc_cfg_t* XCONFIG_HW_FindBMC(const xconfig_board_config_t *board,
                                            const str_t *name);
```

### XCONFIG_HW_FindSatellite
查找卫星平台接口配置。
```c
const xconfig_satellite_cfg_t* XCONFIG_HW_FindSatellite(const xconfig_board_config_t *board,
                                                        const str_t *name);
```

### XCONFIG_HW_FindSensor
查找传感器外设配置。
```c
const xconfig_sensor_cfg_t* XCONFIG_HW_FindSensor(const xconfig_board_config_t *board,
                                                  const str_t *name);
```

### XCONFIG_HW_FindStorage
查找存储设备配置。
```c
const xconfig_storage_cfg_t* XCONFIG_HW_FindStorage(const xconfig_board_config_t *board,
                                                    const str_t *name);
```

## 配置数据结构

详细的数据结构定义请参考头文件：
- `xconfig_mcu.h` - MCU外设配置
- `xconfig_bmc.h` - BMC外设配置
- `xconfig_satellite.h` - 卫星平台配置
- `xconfig_sensor.h` - 传感器配置
- `xconfig_storage.h` - 存储设备配置

完整的API文档请参考 [完整README](README.md)。
