// 题目模块：该文件负责具体的数据结构、接口或业务逻辑
package com.oj.problem.common;

// 统一响应体，code 用业务错误码而不是直接复用 HTTP 状态码，方便前端区分具体错误类型
public class ApiResponse<T> {

    // 内部实现细节，避免直接暴露给外部调用方
    private final int code;
    // 内部实现细节，避免直接暴露给外部调用方
    private final String message;
    // 内部实现细节，避免直接暴露给外部调用方
    private final T data;

    // 内部实现细节，避免直接暴露给外部调用方
    private ApiResponse(int code, String message, T data) {
        this.code = code;
        this.message = message;
        this.data = data;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public static <T> ApiResponse<T> success(T data) {
        // 返回本阶段计算结果，供上层流程继续使用
        return new ApiResponse<>(200, "success", data);
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public static <T> ApiResponse<T> success(String message, T data) {
        // 返回本阶段计算结果，供上层流程继续使用
        return new ApiResponse<>(200, message, data);
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public static <T> ApiResponse<T> created(String message, T data) {
        // 返回本阶段计算结果，供上层流程继续使用
        return new ApiResponse<>(201, message, data);
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public static <T> ApiResponse<T> failure(int code, String message) {
        // 返回本阶段计算结果，供上层流程继续使用
        return new ApiResponse<>(code, message, null);
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public int getCode() {
        // 返回本阶段计算结果，供上层流程继续使用
        return code;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public String getMessage() {
        // 返回本阶段计算结果，供上层流程继续使用
        return message;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public T getData() {
        // 返回本阶段计算结果，供上层流程继续使用
        return data;
    }
}
