package com.oj.problem.dto.response;

import java.time.LocalDateTime;

public class ProblemStatusResponse {

    // 用户在单个题目上的做题状态记录
    private Long userId;
    // 题目 ID，用于定位本次提交或状态更新关联的题目。
    private Long problemId;
    // 是否已经通过该题，历史上任意一次 AC 后保持为 true。
    private Boolean accepted;
    // 历史最高分，用于部分通过的题目展示用户最好成绩。
    private Integer bestScore;
    // 最近一次提交得分，便于用户判断当前进展。
    private Integer lastScore;
    // 本题当前评测总分，和 score 一起展示分数占比。
    private Integer maxScore;
    // 最近一次提交时间，用于题目列表和状态详情展示。
    private LocalDateTime lastSubmittedAt;
    // 首次通过时间，只有从未通过变为通过时写入。
    private LocalDateTime acceptedAt;

    public Long getUserId() {
        return userId;
    }

    public void setUserId(Long userId) {
        this.userId = userId;
    }

    public Long getProblemId() {
        return problemId;
    }

    public void setProblemId(Long problemId) {
        this.problemId = problemId;
    }

    public Boolean getAccepted() {
        return accepted;
    }

    public void setAccepted(Boolean accepted) {
        this.accepted = accepted;
    }

    public Integer getBestScore() {
        return bestScore;
    }

    public void setBestScore(Integer bestScore) {
        this.bestScore = bestScore;
    }

    public Integer getLastScore() {
        return lastScore;
    }

    public void setLastScore(Integer lastScore) {
        this.lastScore = lastScore;
    }

    public Integer getMaxScore() {
        return maxScore;
    }

    public void setMaxScore(Integer maxScore) {
        this.maxScore = maxScore;
    }

    public LocalDateTime getLastSubmittedAt() {
        return lastSubmittedAt;
    }

    public void setLastSubmittedAt(LocalDateTime lastSubmittedAt) {
        this.lastSubmittedAt = lastSubmittedAt;
    }

    public LocalDateTime getAcceptedAt() {
        return acceptedAt;
    }

    public void setAcceptedAt(LocalDateTime acceptedAt) {
        this.acceptedAt = acceptedAt;
    }
}
