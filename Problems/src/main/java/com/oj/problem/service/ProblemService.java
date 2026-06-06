// 题目模块：该文件负责具体的数据结构、接口或业务逻辑
package com.oj.problem.service;

import com.oj.problem.dto.request.ProblemQueryRequest;
import com.oj.problem.dto.request.ProblemUpsertRequest;
import com.oj.problem.dto.response.ProblemDetailResponse;
import com.oj.problem.dto.response.ProblemMutationResponse;
import com.oj.problem.dto.response.ProblemPageResponse;
import com.oj.problem.dto.response.TestCaseResponse;
import com.oj.problem.security.CurrentUser;
import java.util.List;

// 对外暴露的方法或字段，通常承接模块间协作
public interface ProblemService {

    ProblemPageResponse listProblems(ProblemQueryRequest queryRequest);

    ProblemDetailResponse getProblemDetail(Long id);

    List<TestCaseResponse> getProblemTestCases(Long id);

    ProblemMutationResponse createProblem(ProblemUpsertRequest request, CurrentUser currentUser);

    ProblemMutationResponse updateProblem(Long id, ProblemUpsertRequest request, CurrentUser currentUser);

    // 评测完成后用这个接口回写提交统计
    void recordSubmissionResult(Long id, boolean accepted);

    void deleteProblem(Long id, CurrentUser currentUser);
}
