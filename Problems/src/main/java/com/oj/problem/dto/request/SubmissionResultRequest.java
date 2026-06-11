package com.oj.problem.dto.request;

import java.time.LocalDateTime;
import javax.validation.constraints.Min;
import javax.validation.constraints.NotNull;

public class SubmissionResultRequest {

    // 评测模块回写结果时用这个 DTO
    @NotNull(message = "用户ID不能为空")
    private Long userId;

    // 题目 ID，用于定位本次提交或状态更新关联的题目。
    @NotNull(message = "题目ID不能为空")
    private Long problemId;

    // 是否已经通过该题，历史上任意一次 AC 后保持为 true。
    @NotNull(message = "是否通过不能为空")
    private Boolean accepted;

    // 单个测试点分值，评测结果会累加为 total_score。
    @Min(value = 0, message = "得分不能小于0")
    private Integer score = 0;

    // 本题当前评测总分，和 score 一起展示分数占比。
    @Min(value = 0, message = "最高分不能小于0")
    private Integer maxScore;

    // 评测模块回写的提交完成时间，状态页按它展示最近提交。
    private LocalDateTime submittedAt;

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

    public Integer getScore() {
        return score;
    }

    public void setScore(Integer score) {
        this.score = score;
    }

    public Integer getMaxScore() {
        return maxScore;
    }

    public void setMaxScore(Integer maxScore) {
        this.maxScore = maxScore;
    }

    public LocalDateTime getSubmittedAt() {
        return submittedAt;
    }

    public void setSubmittedAt(LocalDateTime submittedAt) {
        this.submittedAt = submittedAt;
    }
}
