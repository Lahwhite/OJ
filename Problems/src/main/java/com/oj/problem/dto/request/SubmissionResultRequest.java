package com.oj.problem.dto.request;

import java.time.LocalDateTime;
import javax.validation.constraints.Min;
import javax.validation.constraints.NotNull;

public class SubmissionResultRequest {

    @NotNull(message = "用户ID不能为空")
    private Long userId;

    @NotNull(message = "题目ID不能为空")
    private Long problemId;

    @NotNull(message = "是否通过不能为空")
    private Boolean accepted;

    @Min(value = 0, message = "得分不能小于0")
    private Integer score = 0;

    @Min(value = 0, message = "最高分不能小于0")
    private Integer maxScore;

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
