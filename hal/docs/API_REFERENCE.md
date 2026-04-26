# HAL API 参考手册

## CAN驱动接口

### HAL_CAN_Init
初始化CAN驱动。
```c
int32_t HAL_CAN_Init(const hal_can_config_t *config, hal_can_handle_t *handle);
```

### HAL_CAN_Deinit
关闭CAN驱动。
```c
int32_t HAL_CAN_Deinit(hal_can_handle_t *handle);
```

### HAL_CAN_Send
发送CAN帧。
```c
int32_t HAL_CAN_Send(hal_can_handle_t *handle, const hal_can_frame_t *frame, uint32_t timeout_ms);
```

### HAL_CAN_Receive
接收CAN帧。
```c
int32_t HAL_CAN_Receive(hal_can_handle_t *handle, hal_can_frame_t *frame, uint32_t timeout_ms);
```

### HAL_CAN_SetFilter
设置CAN ID过滤器。
```c
int32_t HAL_CAN_SetFilter(hal_can_handle_t *handle, uint32_t filter_id, uint32_t filter_mask);
```

### HAL_CAN_GetStats
获取统计信息。
```c
int32_t HAL_CAN_GetStats(hal_can_handle_t *handle, hal_can_stats_t *stats);
```

## 串口驱动接口

### HAL_Serial_Init
初始化串口驱动。
```c
int32_t HAL_Serial_Init(const hal_serial_config_t *config, hal_serial_handle_t *handle);
```

### HAL_Serial_Deinit
关闭串口驱动。
```c
int32_t HAL_Serial_Deinit(hal_serial_handle_t *handle);
```

### HAL_Serial_Send
发送数据。
```c
int32_t HAL_Serial_Send(hal_serial_handle_t *handle, const uint8_t *data, uint32_t size, uint32_t timeout_ms);
```

### HAL_Serial_Receive
接收数据。
```c
int32_t HAL_Serial_Receive(hal_serial_handle_t *handle, uint8_t *buffer, uint32_t size, uint32_t timeout_ms);
```

### HAL_Serial_Configure
重新配置串口参数。
```c
int32_t HAL_Serial_Configure(hal_serial_handle_t *handle, const hal_serial_config_t *config);
```

详细参数说明请参考头文件 `hal_can.h` 和 `hal_serial.h`。
