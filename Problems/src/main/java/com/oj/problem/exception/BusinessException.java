// 题目模块：该文件负责具体的数据结构、接口或业务逻辑
package com.oj.problem.exception;

import org.springframework.http.HttpStatus;

// 对外暴露的方法或字段，通常承接模块间协作
public class BusinessException extends RuntimeException {

    // 内部实现细节，避免直接暴露给外部调用方
    private final int code;
    // 内部实现细节，避免直接暴露给外部调用方
    private final HttpStatus httpStatus;

    // code 给前端判断用，httpStatus 给 Spring 转成 HTTP 状态码
    public BusinessException(int code, String message, HttpStatus httpStatus) {
        super(message);
        this.code = code;
        this.httpStatus = httpStatus;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public int getCode() {
        // 返回本阶段计算结果，供上层流程继续使用
        return code;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public HttpStatus getHttpStatus() {
        // 返回本阶段计算结果，供上层流程继续使用
        return httpStatus;
    }
}
