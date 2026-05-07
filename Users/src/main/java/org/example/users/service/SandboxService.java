package org.example.users.service;

import org.example.users.dto.CodeRunRequest;
import org.example.users.dto.CodeRunResponse;

public interface SandboxService {
    CodeRunResponse runCode(CodeRunRequest request);
}
