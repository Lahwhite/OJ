package com.oj.problem.dto.request;

import javax.validation.constraints.NotBlank;
import javax.validation.constraints.Pattern;

public class SubmitCodeRequest {

    // 提交人用户名，会用于评测临时文件名前缀和用户状态记录。
    @NotBlank(message = "用户名不能为空")
    private String username;

    // 提交语言标识，必须能映射到评测引擎支持的源文件名。
    @NotBlank(message = "编程语言不能为空")
    @Pattern(regexp = "^(c|cpp|java|python)$", message = "不支持的编程语言")
    private String language;

    // 业务响应码，成功固定为 0，异常使用模块内错误码。
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
