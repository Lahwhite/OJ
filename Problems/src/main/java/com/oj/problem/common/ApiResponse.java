package com.oj.problem.common;

// 统一响应体，code 用业务错误码而不是直接复用 HTTP 状态码，方便前端区分具体错误类型
public class ApiResponse<T> {

    // 业务响应码，成功固定为 0，异常使用模块内错误码。
    private final int code;
    // 提交接口给前端展示的结果摘要。
    private final String message;
    // 实际响应数据，错误响应中通常为空。
    private final T data;

    private ApiResponse(int code, String message, T data) {
        this.code = code;
        this.message = message;
        this.data = data;
    }

    public static <T> ApiResponse<T> success(T data) {
        return new ApiResponse<>(200, "success", data);
    }

    public static <T> ApiResponse<T> success(String message, T data) {
        return new ApiResponse<>(200, message, data);
    }

    public static <T> ApiResponse<T> created(String message, T data) {
        return new ApiResponse<>(201, message, data);
    }

    public static <T> ApiResponse<T> failure(int code, String message) {
        return new ApiResponse<>(code, message, null);
    }

    public int getCode() {
        return code;
    }

    public String getMessage() {
        return message;
    }

    public T getData() {
        return data;
    }
}
