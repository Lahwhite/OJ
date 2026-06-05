package com.oj.problem.exception;

import org.springframework.http.HttpStatus;

public class BusinessException extends RuntimeException {

    private final int code;
    private final HttpStatus httpStatus;

    // code 给前端判断用，httpStatus 给 Spring 转成 HTTP 状态码
    public BusinessException(int code, String message, HttpStatus httpStatus) {
        super(message);
        this.code = code;
        this.httpStatus = httpStatus;
    }

    public int getCode() {
        return code;
    }

    public HttpStatus getHttpStatus() {
        return httpStatus;
    }
}
