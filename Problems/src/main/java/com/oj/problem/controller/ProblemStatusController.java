package com.oj.problem.controller;

import com.oj.problem.common.ApiResponse;
import com.oj.problem.dto.request.SubmissionResultRequest;
import com.oj.problem.dto.response.ProblemStatusListResponse;
import com.oj.problem.dto.response.ProblemStatusResponse;
import com.oj.problem.service.ProblemStatusService;
import javax.validation.Valid;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.http.HttpStatus;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestHeader;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.server.ResponseStatusException;

@RestController
@RequestMapping("/v1/problem-status")
public class ProblemStatusController {

    private final ProblemStatusService problemStatusService;
    private final String serviceToken;

    public ProblemStatusController(
            ProblemStatusService problemStatusService,
            @Value("${problem.status.service-token}") String serviceToken) {
        this.problemStatusService = problemStatusService;
        this.serviceToken = serviceToken;
    }

    @PutMapping
    public ApiResponse<ProblemStatusResponse> upsertStatus(
            @RequestHeader(value = "X-Service-Token", required = false) String token,
            @Valid @RequestBody SubmissionResultRequest request) {
        requireServiceToken(token);
        return ApiResponse.success("状态已更新", problemStatusService.upsertStatus(request));
    }

    @GetMapping("/users/{userId}")
    public ApiResponse<ProblemStatusListResponse> getUserStatuses(@PathVariable Long userId) {
        return ApiResponse.success(problemStatusService.getUserStatuses(userId));
    }

    @GetMapping("/users/{userId}/problems/{problemId}")
    public ApiResponse<ProblemStatusResponse> getUserProblemStatus(
            @PathVariable Long userId,
            @PathVariable Long problemId) {
        return ApiResponse.success(problemStatusService.getUserProblemStatus(userId, problemId));
    }

    private void requireServiceToken(String token) {
        if (serviceToken == null || serviceToken.isEmpty() || !serviceToken.equals(token)) {
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "Invalid service token");
        }
    }
}
