package org.example.users.dto;

import lombok.Data;

@Data
public class CodeRunRequest {
    private String code;
    private String language;
}
