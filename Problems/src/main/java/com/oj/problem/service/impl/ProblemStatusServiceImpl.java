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

    private final ProblemRepository problemRepository;
    private final ProblemUserStatusRepository statusRepository;

    public ProblemStatusServiceImpl(
            ProblemRepository problemRepository,
            ProblemUserStatusRepository statusRepository) {
        this.problemRepository = problemRepository;
        this.statusRepository = statusRepository;
    }

    @Override
    @Transactional
    public ProblemStatusResponse upsertStatus(SubmissionResultRequest request) {
        ProblemEntity problem = problemRepository.findById(request.getProblemId())
                .orElseThrow(() -> new BusinessException(404002, "题目不存在", HttpStatus.NOT_FOUND));

        ProblemUserStatusEntity status = statusRepository
                .findByUserIdAndProblem_Id(request.getUserId(), request.getProblemId())
                .orElseGet(ProblemUserStatusEntity::new);

        if (status.getId() == null) {
            status.setUserId(request.getUserId());
            status.setProblem(problem);
        }

        LocalDateTime submittedAt = request.getSubmittedAt() == null ? LocalDateTime.now() : request.getSubmittedAt();
        int currentBest = status.getBestScore() == null ? 0 : status.getBestScore();
        int score = request.getScore() == null ? 0 : request.getScore();
        boolean accepted = Boolean.TRUE.equals(request.getAccepted());
        boolean firstAccepted = accepted && !Boolean.TRUE.equals(status.getAccepted());

        status.setAccepted(Boolean.TRUE.equals(status.getAccepted()) || accepted);
        status.setBestScore(Math.max(currentBest, score));
        status.setLastScore(score);
        status.setMaxScore(request.getMaxScore());
        status.setLastSubmittedAt(submittedAt);
        if (firstAccepted) {
            status.setAcceptedAt(submittedAt);
        }

        return toResponse(statusRepository.save(status));
    }

    @Override
    @Transactional(readOnly = true)
    public ProblemStatusListResponse getUserStatuses(Long userId) {
        ProblemStatusListResponse response = new ProblemStatusListResponse();
        response.setUserId(userId);
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
