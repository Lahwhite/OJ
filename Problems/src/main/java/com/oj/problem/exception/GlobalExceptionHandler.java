// 题目模块：该文件负责具体的数据结构、接口或业务逻辑
package com.oj.problem.exception;

import com.oj.problem.common.ApiResponse;
import java.util.stream.Collectors;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.HttpMediaTypeNotSupportedException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.validation.FieldError;
import org.springframework.web.bind.MethodArgumentNotValidException;
import org.springframework.web.bind.MissingRequestHeaderException;
import org.springframework.web.bind.annotation.ExceptionHandler;
import org.springframework.web.bind.annotation.RestControllerAdvice;
import org.springframework.web.server.ResponseStatusException;

@RestControllerAdvice
// 对外暴露的方法或字段，通常承接模块间协作
public class GlobalExceptionHandler {

    // 内部实现细节，避免直接暴露给外部调用方
    private static final Logger LOGGER = LoggerFactory.getLogger(GlobalExceptionHandler.class);

    @ExceptionHandler(BusinessException.class)
    // 对外暴露的方法或字段，通常承接模块间协作
    public ResponseEntity<ApiResponse<Void>> handleBusinessException(BusinessException ex) {
        // 返回本阶段计算结果，供上层流程继续使用
        return ResponseEntity.status(ex.getHttpStatus())
                .body(ApiResponse.failure(ex.getCode(), ex.getMessage()));
    }

    // 处理参数校验失败的情况，把所有字段错误拼成一条消息返回
    @ExceptionHandler(MethodArgumentNotValidException.class)
    public ResponseEntity<ApiResponse<Void>> handleValidationException(MethodArgumentNotValidException ex) {
        String message = ex.getBindingResult().getFieldErrors().stream()
                .map(FieldError::getDefaultMessage)
                .collect(Collectors.joining("; "));
        // 返回本阶段计算结果，供上层流程继续使用
        return ResponseEntity.status(HttpStatus.BAD_REQUEST)
                .body(ApiResponse.failure(400001, message));
    }

    @ExceptionHandler(ResponseStatusException.class)
    // 对外暴露的方法或字段，通常承接模块间协作
    public ResponseEntity<ApiResponse<Void>> handleResponseStatusException(ResponseStatusException ex) {
        int code = ex.getStatus() == HttpStatus.UNAUTHORIZED ? 401003 : 500001;
        // 返回本阶段计算结果，供上层流程继续使用
        return ResponseEntity.status(ex.getStatus())
                .body(ApiResponse.failure(code, ex.getReason()));
    }

    @ExceptionHandler(HttpMediaTypeNotSupportedException.class)
    // 对外暴露的方法或字段，通常承接模块间协作
    public ResponseEntity<ApiResponse<Void>> handleUnsupportedMediaTypeException(HttpMediaTypeNotSupportedException ex) {
        // 返回本阶段计算结果，供上层流程继续使用
        return ResponseEntity.status(HttpStatus.UNSUPPORTED_MEDIA_TYPE)
                .body(ApiResponse.failure(415001, "请求格式不受支持，请使用 application/json"));
    }

    @ExceptionHandler(MissingRequestHeaderException.class)
    // 对外暴露的方法或字段，通常承接模块间协作
    public ResponseEntity<ApiResponse<Void>> handleMissingRequestHeaderException(MissingRequestHeaderException ex) {
        // 返回本阶段计算结果，供上层流程继续使用
        return ResponseEntity.status(HttpStatus.UNAUTHORIZED)
                .body(ApiResponse.failure(401003, "Missing authorization header"));
    }

    @ExceptionHandler(IllegalArgumentException.class)
    // 对外暴露的方法或字段，通常承接模块间协作
    public ResponseEntity<ApiResponse<Void>> handleIllegalArgumentException(IllegalArgumentException ex) {
        // 条件分支：根据当前输入或状态选择不同处理路径
        if ("权限不足".equals(ex.getMessage())) {
            // 返回本阶段计算结果，供上层流程继续使用
            return ResponseEntity.status(HttpStatus.FORBIDDEN)
                    .body(ApiResponse.failure(403001, ex.getMessage()));
        }
        // 返回本阶段计算结果，供上层流程继续使用
        return ResponseEntity.status(HttpStatus.UNAUTHORIZED)
                .body(ApiResponse.failure(401003, ex.getMessage()));
    }

    @ExceptionHandler(Exception.class)
    // 对外暴露的方法或字段，通常承接模块间协作
    public ResponseEntity<ApiResponse<Void>> handleUnexpectedException(Exception ex) {
        LOGGER.error("Unhandled request exception", ex);
        // 返回本阶段计算结果，供上层流程继续使用
        return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR)
                .body(ApiResponse.failure(500001, "服务器内部错误"));
    }
}
