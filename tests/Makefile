# CSPD-BSP 单元测试 Makefile
# 无外部依赖，纯C实现

# 编译器设置
CC = gcc
CFLAGS = -Wall -Wextra -g -std=c99 -D_GNU_SOURCE
LDFLAGS = -lpthread

# 目录设置
ROOT_DIR = ..
OSAL_DIR = $(ROOT_DIR)/osal
HAL_DIR = $(ROOT_DIR)/hal
SERVICE_DIR = $(ROOT_DIR)/service
APPS_DIR = $(ROOT_DIR)/apps

# 包含路径
INCLUDES = -I. \
           -I$(OSAL_DIR)/inc \
           -I$(HAL_DIR)/inc \
           -I$(SERVICE_DIR)/inc \
           -I$(APPS_DIR)/protocol_converter/inc \
           -I$(APPS_DIR)/can_gateway/inc

# 源文件
OSAL_SOURCES = $(wildcard $(OSAL_DIR)/linux/*.c)
HAL_SOURCES = $(wildcard $(HAL_DIR)/linux/*.c)
SERVICE_SOURCES = $(wildcard $(SERVICE_DIR)/linux/*.c)
APPS_GATEWAY_SOURCES = $(wildcard $(APPS_DIR)/can_gateway/*.c)
APPS_PROTOCOL_SOURCES = $(wildcard $(APPS_DIR)/protocol_converter/*.c)

# 测试可执行文件
TESTS = test_os_task \
        test_os_queue \
        test_os_mutex \
        test_hal_can \
        test_payload_service \
        test_can_gateway

# 默认目标
all: $(TESTS)

# OSAL测试
test_os_task: osal/test_os_task.c $(OSAL_SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS)

test_os_queue: osal/test_os_queue.c $(OSAL_SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS)

test_os_mutex: osal/test_os_mutex.c $(OSAL_SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS)

# HAL测试
test_hal_can: hal/test_hal_can.c $(HAL_SOURCES) $(OSAL_SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS)

# Service测试
test_payload_service: service/test_payload_service.c $(SERVICE_SOURCES) $(HAL_SOURCES) $(OSAL_SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS)

# Apps测试
test_can_gateway: apps/test_can_gateway.c $(APPS_GATEWAY_SOURCES) $(SERVICE_SOURCES) $(HAL_SOURCES) $(OSAL_SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS)

# 运行所有测试
test: all
	@echo ""
	@echo "========================================"
	@echo "Running All Tests"
	@echo "========================================"
	@for test in $(TESTS); do \
		echo ""; \
		echo "Running $$test..."; \
		./$$test || exit 1; \
	done
	@echo ""
	@echo "========================================"
	@echo "All Tests Passed!"
	@echo "========================================"

# 运行单个测试
run-%: %
	./$<

# 清理
clean:
	rm -f $(TESTS)
	rm -f *.o
	rm -rf build

# 帮助
help:
	@echo "CSPD-BSP Test Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all                - Build all tests"
	@echo "  test               - Build and run all tests"
	@echo "  run-<test_name>    - Run specific test"
	@echo "  clean              - Remove build artifacts"
	@echo ""
	@echo "Available tests:"
	@for test in $(TESTS); do \
		echo "  - $$test"; \
	done

.PHONY: all test clean help
