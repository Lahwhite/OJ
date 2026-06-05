// 题目模块：该文件负责具体的数据结构、接口或业务逻辑
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
// 对外暴露的方法或字段，通常承接模块间协作
public class ProblemStatusController {

    // 内部实现细节，避免直接暴露给外部调用方
    private final ProblemStatusService problemStatusService;
    // 内部实现细节，避免直接暴露给外部调用方
    private final String serviceToken;

    // 对外暴露的方法或字段，通常承接模块间协作
    public ProblemStatusController(
            ProblemStatusService problemStatusService,
            @Value("${problem.status.service-token}") String serviceToken) {
        this.problemStatusService = problemStatusService;
        this.serviceToken = serviceToken;
    }

    // 评测结果回写走这个接口，外部模块要带 service token
    @PutMapping
    public ApiResponse<ProblemStatusResponse> upsertStatus(
            @RequestHeader(value = "X-Service-Token", required = false) String token,
            @Valid @RequestBody SubmissionResultRequest request) {
        requireServiceToken(token);
        // 返回本阶段计算结果，供上层流程继续使用
        return ApiResponse.success("状态已更新", problemStatusService.upsertStatus(request));
    }

    @GetMapping("/users/{userId}")
    // 对外暴露的方法或字段，通常承接模块间协作
    public ApiResponse<ProblemStatusListResponse> getUserStatuses(@PathVariable Long userId) {
        // 返回本阶段计算结果，供上层流程继续使用
        return ApiResponse.success(problemStatusService.getUserStatuses(userId));
    }

    @GetMapping("/users/{userId}/problems/{problemId}")
    // 对外暴露的方法或字段，通常承接模块间协作
    public ApiResponse<ProblemStatusResponse> getUserProblemStatus(
            @PathVariable Long userId,
            @PathVariable Long problemId) {
        // 返回本阶段计算结果，供上层流程继续使用
        return ApiResponse.success(problemStatusService.getUserProblemStatus(userId, problemId));
    }

    // 内部实现细节，避免直接暴露给外部调用方
    private void requireServiceToken(String token) {
        // 条件分支：根据当前输入或状态选择不同处理路径
        if (serviceToken == null || serviceToken.isEmpty() || !serviceToken.equals(token)) {
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "Invalid service token");
        }
    }
}
