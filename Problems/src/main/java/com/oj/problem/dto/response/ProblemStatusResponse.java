// 题目模块：该文件负责具体的数据结构、接口或业务逻辑
package com.oj.problem.dto.response;

import java.time.LocalDateTime;

// 对外暴露的方法或字段，通常承接模块间协作
public class ProblemStatusResponse {

    // 用户在单个题目上的做题状态记录
    private Long userId;
    private Long problemId;
    // 内部实现细节，避免直接暴露给外部调用方
    private Boolean accepted;
    // 内部实现细节，避免直接暴露给外部调用方
    private Integer bestScore;
    // 内部实现细节，避免直接暴露给外部调用方
    private Integer lastScore;
    // 内部实现细节，避免直接暴露给外部调用方
    private Integer maxScore;
    // 内部实现细节，避免直接暴露给外部调用方
    private LocalDateTime lastSubmittedAt;
    // 内部实现细节，避免直接暴露给外部调用方
    private LocalDateTime acceptedAt;

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
    public Integer getBestScore() {
        // 返回本阶段计算结果，供上层流程继续使用
        return bestScore;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setBestScore(Integer bestScore) {
        this.bestScore = bestScore;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public Integer getLastScore() {
        // 返回本阶段计算结果，供上层流程继续使用
        return lastScore;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setLastScore(Integer lastScore) {
        this.lastScore = lastScore;
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
    public LocalDateTime getLastSubmittedAt() {
        // 返回本阶段计算结果，供上层流程继续使用
        return lastSubmittedAt;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setLastSubmittedAt(LocalDateTime lastSubmittedAt) {
        this.lastSubmittedAt = lastSubmittedAt;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public LocalDateTime getAcceptedAt() {
        // 返回本阶段计算结果，供上层流程继续使用
        return acceptedAt;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setAcceptedAt(LocalDateTime acceptedAt) {
        this.acceptedAt = acceptedAt;
    }
}
