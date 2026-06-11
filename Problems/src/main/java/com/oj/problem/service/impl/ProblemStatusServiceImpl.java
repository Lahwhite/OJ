// 题目模块：维护用户与题目的做题状态
package com.oj.problem.service.impl;

import com.oj.problem.dto.request.SubmissionResultRequest;
import com.oj.problem.dto.response.ProblemStatusListResponse;
import com.oj.problem.dto.response.ProblemStatusResponse;
import com.oj.problem.entity.ProblemEntity;
import com.oj.problem.entity.ProblemUserStatusEntity;
import com.oj.problem.exception.BusinessException;
import com.oj.problem.repository.ProblemRepository;
import com.oj.problem.repository.ProblemUserStatusRepository;
import com.oj.problem.service.ProblemStatusService;
import java.time.LocalDateTime;
import java.util.stream.Collectors;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

@Service
public class ProblemStatusServiceImpl implements ProblemStatusService {

    // 用于确认提交对应的题目是否存在
    private final ProblemRepository problemRepository;
    // 负责持久化用户的 AC 状态、得分和提交时间
    private final ProblemUserStatusRepository statusRepository;

    public ProblemStatusServiceImpl(
            ProblemRepository problemRepository,
            ProblemUserStatusRepository statusRepository) {
        this.problemRepository = problemRepository;
        this.statusRepository = statusRepository;
    }

    // 评测结果回写时，如果没有记录就新建，有记录就更新
    @Override
    @Transactional
    public ProblemStatusResponse upsertStatus(SubmissionResultRequest request) {
        ProblemEntity problem = problemRepository.findById(request.getProblemId())
                .orElseThrow(() -> new BusinessException(404002, "题目不存在", HttpStatus.NOT_FOUND));

        ProblemUserStatusEntity status = statusRepository
                .findByUserIdAndProblem_Id(request.getUserId(), request.getProblemId())
                .orElseGet(ProblemUserStatusEntity::new);

        // 第一次提交这道题时，先补齐用户和题目关联
        if (status.getId() == null) {
            status.setUserId(request.getUserId());
            status.setProblem(problem);
        }

        LocalDateTime submittedAt = request.getSubmittedAt() == null ? LocalDateTime.now() : request.getSubmittedAt();
        int currentBest = status.getBestScore() == null ? 0 : status.getBestScore();
        int score = request.getScore() == null ? 0 : request.getScore();
        boolean accepted = Boolean.TRUE.equals(request.getAccepted());
        boolean firstAccepted = accepted && !Boolean.TRUE.equals(status.getAccepted());

        // 只要曾经通过过，这个状态就保持为 true
        status.setAccepted(Boolean.TRUE.equals(status.getAccepted()) || accepted);
        // bestScore 记录历史最好成绩，避免被更差的提交覆盖
        status.setBestScore(Math.max(currentBest, score));
        // lastScore 保留最近一次评测结果，方便前端展示
        status.setLastScore(score);
        status.setMaxScore(request.getMaxScore());
        status.setLastSubmittedAt(submittedAt);
        // 首次 AC 单独记录时间，便于后续统计或排行使用
        if (firstAccepted) {
            status.setAcceptedAt(submittedAt);
        }

        return toResponse(statusRepository.save(status));
    }
    // 按用户聚合题目状态，供题库列表批量标记 AC 情况。

    @Override
    @Transactional(readOnly = true)
    public ProblemStatusListResponse getUserStatuses(Long userId) {
        ProblemStatusListResponse response = new ProblemStatusListResponse();
        response.setUserId(userId);
        // 按题号升序返回，前端列表页更容易直接使用
        response.setStatuses(statusRepository.findByUserIdOrderByProblem_IdAsc(userId).stream()
                .map(this::toResponse)
                .collect(Collectors.toList()));
        return response;
    }

    @Override
    @Transactional(readOnly = true)
    public ProblemStatusResponse getUserProblemStatus(Long userId, Long problemId) {
        return statusRepository.findByUserIdAndProblem_Id(userId, problemId)
                .map(this::toResponse)
                .orElseGet(() -> emptyResponse(userId, problemId));
    }

    // 如果用户还没提交过这题，就返回一份默认状态给前端
    private ProblemStatusResponse emptyResponse(Long userId, Long problemId) {
        ProblemStatusResponse response = new ProblemStatusResponse();
        response.setUserId(userId);
        response.setProblemId(problemId);
        response.setAccepted(Boolean.FALSE);
        response.setBestScore(0);
        response.setLastScore(null);
        response.setMaxScore(null);
        return response;
    }

    // 实体转响应对象，统一由这里控制对外字段
    private ProblemStatusResponse toResponse(ProblemUserStatusEntity entity) {
        ProblemStatusResponse response = new ProblemStatusResponse();
        response.setUserId(entity.getUserId());
        response.setProblemId(entity.getProblem().getId());
        response.setAccepted(entity.getAccepted());
        response.setBestScore(entity.getBestScore());
        response.setLastScore(entity.getLastScore());
        response.setMaxScore(entity.getMaxScore());
        response.setLastSubmittedAt(entity.getLastSubmittedAt());
        response.setAcceptedAt(entity.getAcceptedAt());
        return response;
    }
}
