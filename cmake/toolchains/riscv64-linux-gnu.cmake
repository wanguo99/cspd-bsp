# RISC-V 64 Linux 交叉编译工具链

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR riscv64)

# 编译器设置
set(CMAKE_C_COMPILER riscv64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER riscv64-linux-gnu-g++)

# 编译选项
# rv64imafdc = RV64I base + M(乘除法) + A(原子) + F(单精度浮点) + D(双精度浮点) + C(压缩指令)
set(CMAKE_C_FLAGS_INIT "-march=rv64imafdc -mabi=lp64d")
set(CMAKE_CXX_FLAGS_INIT "-march=rv64imafdc -mabi=lp64d")

# 查找路径设置
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
