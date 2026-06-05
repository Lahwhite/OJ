package com.oj.problem.controller;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.when;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.put;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.jsonPath;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.oj.problem.dto.response.ProblemStatusListResponse;
import com.oj.problem.dto.response.ProblemStatusResponse;
import com.oj.problem.exception.GlobalExceptionHandler;
import com.oj.problem.service.ProblemStatusService;
import java.util.Collections;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.extension.ExtendWith;
import org.mockito.Mock;
import org.mockito.junit.jupiter.MockitoExtension;
import org.springframework.http.MediaType;
import org.springframework.test.web.servlet.MockMvc;
import org.springframework.test.web.servlet.setup.MockMvcBuilders;

@ExtendWith(MockitoExtension.class)
class ProblemStatusControllerTest {

    @Mock
    private ProblemStatusService problemStatusService;

    private MockMvc mockMvc;
    private final ObjectMapper objectMapper = new ObjectMapper();

    @BeforeEach
    void setUp() {
        ProblemStatusController controller = new ProblemStatusController(problemStatusService, "secret");
        mockMvc = MockMvcBuilders.standaloneSetup(controller)
                .setControllerAdvice(new GlobalExceptionHandler())
                .build();
    }

    @Test
    void upsertStatusShouldRequireServiceToken() throws Exception {
        mockMvc.perform(put("/v1/problem-status")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content(validRequestJson()))
                .andExpect(status().isUnauthorized());
    }

    @Test
    void upsertStatusShouldReturnUpdatedStatus() throws Exception {
        ProblemStatusResponse response = new ProblemStatusResponse();
        response.setUserId(1L);
        response.setProblemId(2L);
        response.setAccepted(true);
        response.setBestScore(100);
        when(problemStatusService.upsertStatus(any())).thenReturn(response);

        mockMvc.perform(put("/v1/problem-status")
                        .header("X-Service-Token", "secret")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content(validRequestJson()))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.data.userId").value(1))
                .andExpect(jsonPath("$.data.problemId").value(2))
                .andExpect(jsonPath("$.data.accepted").value(true))
                .andExpect(jsonPath("$.data.bestScore").value(100));
    }

    @Test
    void getUserStatusesShouldReturnStatusList() throws Exception {
        ProblemStatusListResponse response = new ProblemStatusListResponse();
        response.setUserId(1L);
        response.setStatuses(Collections.emptyList());
        when(problemStatusService.getUserStatuses(1L)).thenReturn(response);

        mockMvc.perform(get("/v1/problem-status/users/1"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.data.userId").value(1))
                .andExpect(jsonPath("$.data.statuses").isArray());
    }

    private String validRequestJson() throws Exception {
        java.util.Map<String, Object> request = new java.util.LinkedHashMap<>();
        request.put("userId", 1);
        request.put("problemId", 2);
        request.put("accepted", true);
        request.put("score", 100);
        request.put("maxScore", 100);
        return objectMapper.writeValueAsString(request);
    }
}
