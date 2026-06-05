// 题目模块：该文件负责具体的数据结构、接口或业务逻辑
package com.oj.problem.exception;

import static org.junit.jupiter.api.Assertions.assertEquals;

import com.oj.problem.common.ApiResponse;
import org.junit.jupiter.api.Test;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.HttpMediaTypeNotSupportedException;
import org.springframework.web.server.ResponseStatusException;

// 类定义：封装这一部分的职责边界
class GlobalExceptionHandlerTest {

    // 内部实现细节，避免直接暴露给外部调用方
    private final GlobalExceptionHandler handler = new GlobalExceptionHandler();

    @Test
    void handleBusinessExceptionShouldPreserveCodeAndStatus() {
        ResponseEntity<ApiResponse<Void>> response = handler.handleBusinessException(
                new BusinessException(404002, "题目不存在", HttpStatus.NOT_FOUND));

        assertEquals(HttpStatus.NOT_FOUND, response.getStatusCode());
        assertEquals(404002, response.getBody().getCode());
    }

    @Test
    void handleResponseStatusExceptionShouldMapUnauthorizedCode() {
        ResponseEntity<ApiResponse<Void>> response = handler.handleResponseStatusException(
                new ResponseStatusException(HttpStatus.UNAUTHORIZED, "未登录"));

        assertEquals(HttpStatus.UNAUTHORIZED, response.getStatusCode());
        assertEquals(401003, response.getBody().getCode());
    }

    @Test
    void handleIllegalArgumentExceptionShouldMapForbidden() {
        ResponseEntity<ApiResponse<Void>> response = handler.handleIllegalArgumentException(
                new IllegalArgumentException("权限不足"));

        assertEquals(HttpStatus.FORBIDDEN, response.getStatusCode());
        assertEquals(403001, response.getBody().getCode());
    }

    @Test
    void handleUnexpectedExceptionShouldReturnInternalServerError() {
        ResponseEntity<ApiResponse<Void>> response = handler.handleUnexpectedException(new RuntimeException("boom"));

        assertEquals(HttpStatus.INTERNAL_SERVER_ERROR, response.getStatusCode());
        assertEquals(500001, response.getBody().getCode());
    }

    @Test
    void handleUnsupportedMediaTypeExceptionShouldReturnUnsupportedMediaType() {
        ResponseEntity<ApiResponse<Void>> response = handler.handleUnsupportedMediaTypeException(
                new HttpMediaTypeNotSupportedException("text/plain is not supported"));

        assertEquals(HttpStatus.UNSUPPORTED_MEDIA_TYPE, response.getStatusCode());
        assertEquals(415001, response.getBody().getCode());
    }
}
