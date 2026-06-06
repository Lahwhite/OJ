package com.oj.problem.service;

import com.oj.problem.dto.request.SubmitCodeRequest;
import com.oj.problem.dto.response.SubmitCodeResponse;

public interface JudgeService {

    SubmitCodeResponse submit(Long problemId, SubmitCodeRequest request);
}
