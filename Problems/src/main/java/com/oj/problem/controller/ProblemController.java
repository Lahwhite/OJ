// 题目模块：对外提供题目查询、详情和管理接口
package com.oj.problem.controller;

import com.oj.problem.common.ApiResponse;
import com.oj.problem.dto.request.ProblemQueryRequest;
import com.oj.problem.dto.request.ProblemUpsertRequest;
import com.oj.problem.dto.response.ProblemDetailResponse;
import com.oj.problem.dto.response.ProblemMutationResponse;
import com.oj.problem.dto.response.ProblemPageResponse;
import com.oj.problem.dto.response.TestCaseResponse;
import com.oj.problem.security.CurrentUser;
import com.oj.problem.security.JwtTokenService;
import com.oj.problem.service.ProblemService;
import java.util.List;
import javax.validation.Valid;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpStatus;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestHeader;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.ResponseStatus;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/v1/problems")
public class ProblemController {

    // 业务能力统一从 service 层进入，controller 只负责参数接收和结果返回
    private final ProblemService problemService;
    // 管理接口需要先校验管理员身份
    private final JwtTokenService jwtTokenService;

    public ProblemController(ProblemService problemService, JwtTokenService jwtTokenService) {
        this.problemService = problemService;
        this.jwtTokenService = jwtTokenService;
    }

    @GetMapping
    // 题目列表页和筛选功能都走这个接口
    public ApiResponse<ProblemPageResponse> listProblems(@Valid ProblemQueryRequest queryRequest) {
        return ApiResponse.success(problemService.listProblems(queryRequest));
    }

    @GetMapping("/{id}")
    // 返回单道题目的完整题面信息
    public ApiResponse<ProblemDetailResponse> getProblem(@PathVariable Long id) {
        return ApiResponse.success(problemService.getProblemDetail(id));
    }

    @GetMapping("/{id}/test-cases")
    // 需要单独查看测试点时走这个接口
    public ApiResponse<List<TestCaseResponse>> getProblemTestCases(@PathVariable Long id) {
        return ApiResponse.success(problemService.getProblemTestCases(id));
    }

    @PostMapping
    @ResponseStatus(HttpStatus.CREATED)
    // 新建题目只开放给管理员
    public ApiResponse<ProblemMutationResponse> createProblem(
            @RequestHeader(HttpHeaders.AUTHORIZATION) String authorization,
            @Valid @RequestBody ProblemUpsertRequest request) {
        CurrentUser currentUser = requireAdmin(authorization);
        return ApiResponse.created("创建成功", problemService.createProblem(request, currentUser));
    }

    @PutMapping("/{id}")
    // 更新时复用同一套题目入参校验规则
    public ApiResponse<ProblemMutationResponse> updateProblem(
            @PathVariable Long id,
            @RequestHeader(HttpHeaders.AUTHORIZATION) String authorization,
            @Valid @RequestBody ProblemUpsertRequest request) {
        CurrentUser currentUser = requireAdmin(authorization);
        return ApiResponse.success("更新成功", problemService.updateProblem(id, request, currentUser));
    }

    @DeleteMapping("/{id}")
    // 删除题目前同样需要确认管理员身份
    public ApiResponse<Void> deleteProblem(
            @PathVariable Long id,
            @RequestHeader(HttpHeaders.AUTHORIZATION) String authorization) {
        CurrentUser currentUser = requireAdmin(authorization);
        problemService.deleteProblem(id, currentUser);
        return ApiResponse.success("删除成功", null);
    }

    // 统一在这里解析并校验 Authorization 头，避免每个接口重复写校验逻辑
    private CurrentUser requireAdmin(String authorization) {
        return jwtTokenService.requireAdmin(authorization);
    }
}
