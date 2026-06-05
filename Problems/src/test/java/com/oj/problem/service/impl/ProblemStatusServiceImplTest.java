// 题目模块：该文件负责具体的数据结构、接口或业务逻辑
package com.oj.problem.service.impl;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.when;

import com.oj.problem.dto.request.SubmissionResultRequest;
import com.oj.problem.dto.response.ProblemStatusListResponse;
import com.oj.problem.dto.response.ProblemStatusResponse;
import com.oj.problem.entity.ProblemEntity;
import com.oj.problem.entity.ProblemUserStatusEntity;
import com.oj.problem.repository.ProblemRepository;
import com.oj.problem.repository.ProblemUserStatusRepository;
import java.time.LocalDateTime;
import java.util.Arrays;
import java.util.Optional;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.extension.ExtendWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.junit.jupiter.MockitoExtension;

@ExtendWith(MockitoExtension.class)
// 类定义：封装这一部分的职责边界
class ProblemStatusServiceImplTest {

    @Mock
    // 内部实现细节，避免直接暴露给外部调用方
    private ProblemRepository problemRepository;

    @Mock
    // 内部实现细节，避免直接暴露给外部调用方
    private ProblemUserStatusRepository statusRepository;

    @InjectMocks
    // 内部实现细节，避免直接暴露给外部调用方
    private ProblemStatusServiceImpl statusService;

    @Test
    void upsertStatusShouldCreateNewAcceptedStatus() {
        ProblemEntity problem = new ProblemEntity();
        problem.setId(10L);
        LocalDateTime submittedAt = LocalDateTime.of(2026, 6, 4, 20, 0);
        SubmissionResultRequest request = request(1L, 10L, true, 80, 100, submittedAt);

        when(problemRepository.findById(10L)).thenReturn(Optional.of(problem));
        when(statusRepository.findByUserIdAndProblem_Id(1L, 10L)).thenReturn(Optional.empty());
        when(statusRepository.save(any(ProblemUserStatusEntity.class))).thenAnswer(invocation -> invocation.getArgument(0));

        ProblemStatusResponse response = statusService.upsertStatus(request);

        assertEquals(1L, response.getUserId());
        assertEquals(10L, response.getProblemId());
        assertTrue(response.getAccepted());
        assertEquals(80, response.getBestScore());
        assertEquals(80, response.getLastScore());
        assertEquals(100, response.getMaxScore());
        assertEquals(submittedAt, response.getAcceptedAt());
    }

    @Test
    void upsertStatusShouldNotDowngradeAcceptedOrBestScore() {
        ProblemEntity problem = new ProblemEntity();
        problem.setId(11L);
        ProblemUserStatusEntity existing = new ProblemUserStatusEntity();
        existing.setId(5L);
        existing.setUserId(2L);
        existing.setProblem(problem);
        existing.setAccepted(true);
        existing.setBestScore(90);

        SubmissionResultRequest request = request(2L, 11L, false, 70, 100, LocalDateTime.of(2026, 6, 4, 21, 0));

        when(problemRepository.findById(11L)).thenReturn(Optional.of(problem));
        when(statusRepository.findByUserIdAndProblem_Id(2L, 11L)).thenReturn(Optional.of(existing));
        when(statusRepository.save(any(ProblemUserStatusEntity.class))).thenAnswer(invocation -> invocation.getArgument(0));

        ProblemStatusResponse response = statusService.upsertStatus(request);

        assertTrue(response.getAccepted());
        assertEquals(90, response.getBestScore());
        assertEquals(70, response.getLastScore());
    }

    @Test
    void getUserProblemStatusShouldReturnEmptyStatusWhenNoRecordExists() {
        when(statusRepository.findByUserIdAndProblem_Id(3L, 12L)).thenReturn(Optional.empty());

        ProblemStatusResponse response = statusService.getUserProblemStatus(3L, 12L);

        assertEquals(3L, response.getUserId());
        assertEquals(12L, response.getProblemId());
        assertFalse(response.getAccepted());
        assertEquals(0, response.getBestScore());
    }

    @Test
    void getUserStatusesShouldMapAllRows() {
        ProblemEntity first = new ProblemEntity();
        first.setId(1L);
        ProblemEntity second = new ProblemEntity();
        second.setId(2L);
        ProblemUserStatusEntity firstStatus = status(8L, first, true, 100);
        ProblemUserStatusEntity secondStatus = status(8L, second, false, 50);

        when(statusRepository.findByUserIdOrderByProblem_IdAsc(8L))
                .thenReturn(Arrays.asList(firstStatus, secondStatus));

        ProblemStatusListResponse response = statusService.getUserStatuses(8L);

        assertEquals(8L, response.getUserId());
        assertEquals(2, response.getStatuses().size());
        assertEquals(1L, response.getStatuses().get(0).getProblemId());
        assertEquals(2L, response.getStatuses().get(1).getProblemId());
    }

    // 内部实现细节，避免直接暴露给外部调用方
    private SubmissionResultRequest request(
            Long userId,
            Long problemId,
            boolean accepted,
            int score,
            int maxScore,
            LocalDateTime submittedAt) {
        SubmissionResultRequest request = new SubmissionResultRequest();
        request.setUserId(userId);
        request.setProblemId(problemId);
        request.setAccepted(accepted);
        request.setScore(score);
        request.setMaxScore(maxScore);
        request.setSubmittedAt(submittedAt);
        // 返回本阶段计算结果，供上层流程继续使用
        return request;
    }

    // 内部实现细节，避免直接暴露给外部调用方
    private ProblemUserStatusEntity status(Long userId, ProblemEntity problem, boolean accepted, int bestScore) {
        ProblemUserStatusEntity status = new ProblemUserStatusEntity();
        status.setUserId(userId);
        status.setProblem(problem);
        status.setAccepted(accepted);
        status.setBestScore(bestScore);
        // 返回本阶段计算结果，供上层流程继续使用
        return status;
    }
}
