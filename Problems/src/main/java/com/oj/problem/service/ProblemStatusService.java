package com.oj.problem.service;

import com.oj.problem.dto.request.SubmissionResultRequest;
import com.oj.problem.dto.response.ProblemStatusListResponse;
import com.oj.problem.dto.response.ProblemStatusResponse;

public interface ProblemStatusService {

    // 评测结果回写时调用这个接口
    ProblemStatusResponse upsertStatus(SubmissionResultRequest request);

    ProblemStatusListResponse getUserStatuses(Long userId);

    ProblemStatusResponse getUserProblemStatus(Long userId, Long problemId);
}
