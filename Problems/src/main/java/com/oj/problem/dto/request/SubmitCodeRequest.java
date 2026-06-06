package com.oj.problem.dto.request;

import javax.validation.constraints.NotBlank;
import javax.validation.constraints.Pattern;

public class SubmitCodeRequest {

    @NotBlank(message = "用户名不能为空")
    private String username;

    @NotBlank(message = "编程语言不能为空")
    @Pattern(regexp = "^(c|cpp|java|python)$", message = "不支持的编程语言")
    private String language;

    @NotBlank(message = "代码不能为空")
    private String code;

    public String getUsername() {
        return username;
    }

    public void setUsername(String username) {
        this.username = username;
    }

    public String getLanguage() {
        return language;
    }

    public void setLanguage(String language) {
        this.language = language;
    }

    public String getCode() {
        return code;
    }

    public void setCode(String code) {
        this.code = code;
    }
}
