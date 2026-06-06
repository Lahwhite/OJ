package com.oj.problem.controller;

import com.oj.problem.common.ApiResponse;
import com.oj.problem.dto.request.SubmitCodeRequest;
import com.oj.problem.dto.response.SubmitCodeResponse;
import com.oj.problem.service.JudgeService;
import javax.validation.Valid;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/v1/problems")
public class SubmissionController {

    private final JudgeService judgeService;

    public SubmissionController(JudgeService judgeService) {
        this.judgeService = judgeService;
    }

    @PostMapping("/{id}/submit")
    public ApiResponse<SubmitCodeResponse> submitCode(
            @PathVariable Long id,
            @Valid @RequestBody SubmitCodeRequest request) {
        return ApiResponse.success("提交成功", judgeService.submit(id, request));
    }
}
