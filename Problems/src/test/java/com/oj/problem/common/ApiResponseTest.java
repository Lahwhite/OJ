// 题目模块：该文件负责具体的数据结构、接口或业务逻辑
package com.oj.problem.common;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNull;

import org.junit.jupiter.api.Test;

// 类定义：封装这一部分的职责边界
class ApiResponseTest {

    @Test
    void successShouldUseDefaultSuccessMessage() {
        ApiResponse<String> response = ApiResponse.success("ok");
        assertEquals(200, response.getCode());
        assertEquals("success", response.getMessage());
        assertEquals("ok", response.getData());
    }

    @Test
    void successWithCustomMessageShouldKeepPayload() {
        ApiResponse<Integer> response = ApiResponse.success("更新成功", 1);
        assertEquals(200, response.getCode());
        assertEquals("更新成功", response.getMessage());
        assertEquals(Integer.valueOf(1), response.getData());
    }

    @Test
    void createdShouldUse201() {
        ApiResponse<String> response = ApiResponse.created("创建成功", "data");
        assertEquals(201, response.getCode());
        assertEquals("创建成功", response.getMessage());
        assertEquals("data", response.getData());
    }

    @Test
    void failureShouldReturnNullData() {
        ApiResponse<Void> response = ApiResponse.failure(400001, "参数错误");
        assertEquals(400001, response.getCode());
        assertEquals("参数错误", response.getMessage());
        assertNull(response.getData());
    }
}
