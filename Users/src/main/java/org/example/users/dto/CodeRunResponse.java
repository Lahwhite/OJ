package org.example.users.dto;

import lombok.Data;

@Data
public class CodeRunResponse {
    // 运行是否成功
    private Boolean success;
    private String output;
    private String errorMessage;
}
