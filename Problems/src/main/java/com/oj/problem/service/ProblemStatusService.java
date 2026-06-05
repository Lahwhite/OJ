// 题目模块：该文件负责具体的数据结构、接口或业务逻辑
package com.oj.problem.service;

import com.oj.problem.dto.request.SubmissionResultRequest;
import com.oj.problem.dto.response.ProblemStatusListResponse;
import com.oj.problem.dto.response.ProblemStatusResponse;

// 对外暴露的方法或字段，通常承接模块间协作
public interface ProblemStatusService {

    // 评测结果回写时调用这个接口
    ProblemStatusResponse upsertStatus(SubmissionResultRequest request);

    ProblemStatusListResponse getUserStatuses(Long userId);

    ProblemStatusResponse getUserProblemStatus(Long userId, Long problemId);
}
