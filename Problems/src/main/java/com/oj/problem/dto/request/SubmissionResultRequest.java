// 题目模块：该文件负责具体的数据结构、接口或业务逻辑
package com.oj.problem.dto.request;

import java.time.LocalDateTime;
import javax.validation.constraints.Min;
import javax.validation.constraints.NotNull;

// 对外暴露的方法或字段，通常承接模块间协作
public class SubmissionResultRequest {

    // 评测模块回写结果时用这个 DTO
    @NotNull(message = "用户ID不能为空")
    private Long userId;

    @NotNull(message = "题目ID不能为空")
    // 内部实现细节，避免直接暴露给外部调用方
    private Long problemId;

    @NotNull(message = "是否通过不能为空")
    // 内部实现细节，避免直接暴露给外部调用方
    private Boolean accepted;

    @Min(value = 0, message = "得分不能小于0")
    // 内部实现细节，避免直接暴露给外部调用方
    private Integer score = 0;

    @Min(value = 0, message = "最高分不能小于0")
    // 内部实现细节，避免直接暴露给外部调用方
    private Integer maxScore;

    // 内部实现细节，避免直接暴露给外部调用方
    private LocalDateTime submittedAt;

    // 对外暴露的方法或字段，通常承接模块间协作
    public Long getUserId() {
        // 返回本阶段计算结果，供上层流程继续使用
        return userId;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setUserId(Long userId) {
        this.userId = userId;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public Long getProblemId() {
        // 返回本阶段计算结果，供上层流程继续使用
        return problemId;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setProblemId(Long problemId) {
        this.problemId = problemId;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public Boolean getAccepted() {
        // 返回本阶段计算结果，供上层流程继续使用
        return accepted;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setAccepted(Boolean accepted) {
        this.accepted = accepted;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public Integer getScore() {
        // 返回本阶段计算结果，供上层流程继续使用
        return score;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setScore(Integer score) {
        this.score = score;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public Integer getMaxScore() {
        // 返回本阶段计算结果，供上层流程继续使用
        return maxScore;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setMaxScore(Integer maxScore) {
        this.maxScore = maxScore;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public LocalDateTime getSubmittedAt() {
        // 返回本阶段计算结果，供上层流程继续使用
        return submittedAt;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setSubmittedAt(LocalDateTime submittedAt) {
        this.submittedAt = submittedAt;
    }
}
