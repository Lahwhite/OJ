package com.oj.problem.controller;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.when;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.post;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.jsonPath;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.oj.problem.dto.request.SubmitCodeRequest;
import com.oj.problem.dto.response.SubmitCodeResponse;
import com.oj.problem.exception.GlobalExceptionHandler;
import com.oj.problem.service.JudgeService;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.extension.ExtendWith;
import org.mockito.Mock;
import org.mockito.junit.jupiter.MockitoExtension;
import org.springframework.http.MediaType;
import org.springframework.test.web.servlet.MockMvc;
import org.springframework.test.web.servlet.setup.MockMvcBuilders;

@ExtendWith(MockitoExtension.class)
class SubmissionControllerTest {

    @Mock
    private JudgeService judgeService;

    private MockMvc mockMvc;
    private final ObjectMapper objectMapper = new ObjectMapper();

    @BeforeEach
    void setUp() {
        mockMvc = MockMvcBuilders.standaloneSetup(new SubmissionController(judgeService))
                .setControllerAdvice(new GlobalExceptionHandler())
                .build();
    }

    @Test
    void submitCodeShouldReturnSuccess() throws Exception {
        SubmitCodeResponse response = new SubmitCodeResponse();
        response.setUsername("admin");
        response.setProblemId(1L);
        response.setLanguage("java");
        response.setMessage("提交成功，评测结果将由 judge 模块生成");
        when(judgeService.submit(eq(1L), any(SubmitCodeRequest.class))).thenReturn(response);

        SubmitCodeRequest request = new SubmitCodeRequest();
        request.setUsername("admin");
        request.setLanguage("java");
        request.setCode("public class Main { public static void main(String[] args) {} }");

        mockMvc.perform(post("/v1/problems/1/submit")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content(objectMapper.writeValueAsString(request)))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.data.username").value("admin"))
                .andExpect(jsonPath("$.data.problemId").value(1))
                .andExpect(jsonPath("$.data.language").value("java"));
    }

    @Test
    void submitCodeShouldRejectUnsupportedLanguage() throws Exception {
        SubmitCodeRequest request = new SubmitCodeRequest();
        request.setUsername("admin");
        request.setLanguage("javascript");
        request.setCode("console.log(1);");

        mockMvc.perform(post("/v1/problems/1/submit")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content(objectMapper.writeValueAsString(request)))
                .andExpect(status().isBadRequest());
    }
}
