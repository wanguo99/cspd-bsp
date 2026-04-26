# HAL 使用指南

## CAN驱动使用

### 基本使用
```c
#include "hal_can.h"

hal_can_config_t config = {
    .device = "can0",
    .bitrate = 500000,
    .mode = HAL_CAN_MODE_NORMAL
};

hal_can_handle_t handle;
HAL_CAN_Init(&config, &handle);

/* 发送 */
hal_can_frame_t tx_frame = {
    .can_id = 0x123,
    .can_dlc = 8,
    .data = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}
};
HAL_CAN_Send(&handle, &tx_frame, 1000);

/* 接收 */
hal_can_frame_t rx_frame;
HAL_CAN_Receive(&handle, &rx_frame, 5000);

HAL_CAN_Deinit(&handle);
```

### 设置过滤器
```c
/* 只接收ID为0x100-0x1FF的帧 */
HAL_CAN_SetFilter(&handle, 0x100, 0xFF00);
```

## 串口驱动使用

### 基本使用
```c
#include "hal_serial.h"

hal_serial_config_t config = {
    .device = "/dev/ttyS0",
    .baudrate = 115200,
    .data_bits = 8,
    .stop_bits = 1,
    .parity = 'N'
};

hal_serial_handle_t handle;
HAL_Serial_Init(&config, &handle);

/* 发送 */
const uint8_t data[] = "Hello UART\n";
HAL_Serial_Send(&handle, data, sizeof(data), 1000);

/* 接收 */
uint8_t buffer[128];
int32_t len = HAL_Serial_Receive(&handle, buffer, sizeof(buffer), 5000);

HAL_Serial_Deinit(&handle);
```

更多示例请参考测试代码 `tests/src/hal/test_hal_can.c`。
