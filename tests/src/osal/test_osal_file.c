/************************************************************************
 * OSAL文件I/O单元测试
 ************************************************************************/

#include "test_framework.h"
#ifndef STANDALONE_TEST
#include "test_runner.h"
#endif
#include "osal.h"
#include <unistd.h>
#include <string.h>

#define TEST_FILE_PATH "/tmp/osal_test_file.txt"
#define TEST_DATA "Hello OSAL File I/O"

/* 测试前初始化 */
__attribute__((unused)) static void setUp(void)
{
    OS_API_Init();
    /* 清理可能存在的测试文件 */
    unlink(TEST_FILE_PATH);
}

/* 测试后清理 */
__attribute__((unused)) static void tearDown(void)
{
    unlink(TEST_FILE_PATH);
    OS_API_Teardown();
}

/* 测试用例1: 文件打开和关闭 */
void test_osal_file_open_close_success(void)
{
    setUp();
    osal_id_t fd;
    int32 ret;

    /* 创建并打开文件 */
    ret = OSAL_FileOpen(&fd, TEST_FILE_PATH, OS_FILE_MODE_WRITE,
                      OS_FILE_FLAG_CREATE | OS_FILE_FLAG_TRUNCATE);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    TEST_ASSERT_NOT_EQUAL(0, fd);

    /* 关闭文件 */
    ret = OSAL_FileClose(fd);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    tearDown();
}

/* 测试用例2: 文件写入和读取 */
void test_osal_file_write_read_success(void)
{
    setUp();
    osal_id_t fd;
    int32 ret;
    uint8 read_buffer[64];
    uint32 data_len = strlen(TEST_DATA);

    /* 打开文件写入 */
    ret = OSAL_FileOpen(&fd, TEST_FILE_PATH, OS_FILE_MODE_WRITE,
                      OS_FILE_FLAG_CREATE | OS_FILE_FLAG_TRUNCATE);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 写入数据 */
    ret = OSAL_FileWrite(fd, TEST_DATA, data_len);
    TEST_ASSERT_EQUAL((int32)data_len, ret);

    OSAL_FileClose(fd);

    /* 打开文件读取 */
    ret = OSAL_FileOpen(&fd, TEST_FILE_PATH, OS_FILE_MODE_READ, OS_FILE_FLAG_NONE);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 读取数据 */
    memset(read_buffer, 0, sizeof(read_buffer));
    ret = OSAL_FileRead(fd, read_buffer, sizeof(read_buffer));
    TEST_ASSERT_EQUAL((int32)data_len, ret);
    TEST_ASSERT_EQUAL_STRING(TEST_DATA, (char *)read_buffer);

    OSAL_FileClose(fd);
    tearDown();
}

/* 测试用例3: 文件定位 */
void test_osal_file_seek_success(void)
{
    setUp();
    osal_id_t fd;
    int32 ret;
    uint8 buffer[16];
    uint32 data_len = strlen(TEST_DATA);

    /* 创建测试文件 */
    ret = OSAL_FileOpen(&fd, TEST_FILE_PATH, OS_FILE_MODE_RDWR,
                      OS_FILE_FLAG_CREATE | OS_FILE_FLAG_TRUNCATE);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = OSAL_FileWrite(fd, TEST_DATA, data_len);
    TEST_ASSERT_EQUAL((int32)data_len, ret);

    /* 定位到文件开头 */
    ret = OSAL_FileSeek(fd, 0, OS_FILE_SEEK_SET);
    TEST_ASSERT_EQUAL(0, ret);

    /* 读取前6个字节 "Hello " */
    memset(buffer, 0, sizeof(buffer));
    ret = OSAL_FileRead(fd, buffer, 6);
    TEST_ASSERT_EQUAL(6, ret);
    TEST_ASSERT_EQUAL_STRING("Hello ", (char *)buffer);

    /* 定位到文件末尾 */
    ret = OSAL_FileSeek(fd, 0, OS_FILE_SEEK_END);
    TEST_ASSERT_EQUAL((int32)data_len, ret);

    OSAL_FileClose(fd);
    tearDown();
}

/* 测试用例4: 设置非阻塞标志 */
void test_osal_file_setflags_Success(void)
{
    setUp();
    osal_id_t fd;
    int32 ret;
    uint32 flags;

    ret = OSAL_FileOpen(&fd, TEST_FILE_PATH, OS_FILE_MODE_WRITE,
                      OS_FILE_FLAG_CREATE | OS_FILE_FLAG_TRUNCATE);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 设置非阻塞标志 */
    ret = OSAL_FileSetFlags(fd, OS_FILE_FLAG_NONBLOCK);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 获取标志验证 */
    ret = OSAL_FileGetFlags(fd, &flags);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    TEST_ASSERT_TRUE(flags & OS_FILE_FLAG_NONBLOCK);

    OSAL_FileClose(fd);
    tearDown();
}

/* 测试用例5: 无效参数测试 */
void test_osal_file_open_invalidparams(void)
{
    setUp();
    osal_id_t fd;
    int32 ret;

    /* NULL指针 */
    ret = OSAL_FileOpen(NULL, TEST_FILE_PATH, OS_FILE_MODE_READ, OS_FILE_FLAG_NONE);
    TEST_ASSERT_EQUAL(OS_INVALID_POINTER, ret);

    /* NULL路径 */
    ret = OSAL_FileOpen(&fd, NULL, OS_FILE_MODE_READ, OS_FILE_FLAG_NONE);
    TEST_ASSERT_EQUAL(OS_INVALID_POINTER, ret);

    tearDown();
}

/* 测试用例6: 关闭无效文件描述符 */
void test_osal_file_close_invalidiD(void)
{
    setUp();
    int32 ret;

    /* 无效的文件描述符 */
    ret = OSAL_FileClose(0);
    TEST_ASSERT_EQUAL(OS_ERR_INVALID_ID, ret);

    ret = OSAL_FileClose(999);
    TEST_ASSERT_EQUAL(OS_ERR_INVALID_ID, ret);

    tearDown();
}

/* 模块注册 */
#include "test_runner.h"

TEST_MODULE_BEGIN(test_osal_file)
    TEST_CASE(test_osal_file_open_close_success)
    TEST_CASE(test_osal_file_write_read_success)
    TEST_CASE(test_osal_file_seek_success)
    TEST_CASE(test_osal_file_setflags_Success)
    TEST_CASE(test_osal_file_open_invalidparams)
    TEST_CASE(test_osal_file_close_invalidiD)
TEST_MODULE_END(test_osal_file)

/* 独立运行时的主函数 */
#ifdef STANDALONE_TEST
int main(void)
{
    TEST_BEGIN();

    RUN_TEST(test_OSAL_FileOpen_Close_Success);
    RUN_TEST(test_OSAL_FileWrite_Read_Success);
    RUN_TEST(test_OSAL_FileSeek_Success);
    RUN_TEST(test_OSAL_FileSetFlags_Success);
    RUN_TEST(test_OSAL_FileOpen_InvalidParams);
    RUN_TEST(test_OSAL_FileClose_InvalidID);

    return TEST_END();
}
#endif
