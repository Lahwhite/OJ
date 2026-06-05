// 题目模块：该文件负责具体的数据结构、接口或业务逻辑
package com.oj.problem.entity;

import java.time.LocalDateTime;
import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.FetchType;
import javax.persistence.GeneratedValue;
import javax.persistence.GenerationType;
import javax.persistence.Id;
import javax.persistence.Index;
import javax.persistence.JoinColumn;
import javax.persistence.ManyToOne;
import javax.persistence.PrePersist;
import javax.persistence.PreUpdate;
import javax.persistence.Table;
import javax.persistence.UniqueConstraint;

@Entity
@Table(
        name = "problem_user_status",
        uniqueConstraints = @UniqueConstraint(name = "uk_problem_user_status", columnNames = {"user_id", "problem_id"}),
        indexes = {
                @Index(name = "idx_status_user_id", columnList = "user_id"),
                @Index(name = "idx_status_problem_id", columnList = "problem_id"),
                @Index(name = "idx_status_accepted", columnList = "accepted")
        })
// 对外暴露的方法或字段，通常承接模块间协作
public class ProblemUserStatusEntity {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    // 内部实现细节，避免直接暴露给外部调用方
    private Long id;

    // 这里只记用户 id，不再额外连用户表
    @Column(name = "user_id", nullable = false)
    private Long userId;

    @ManyToOne(fetch = FetchType.LAZY, optional = false)
    @JoinColumn(name = "problem_id", nullable = false)
    // 内部实现细节，避免直接暴露给外部调用方
    private ProblemEntity problem;

    @Column(nullable = false)
    // 内部实现细节，避免直接暴露给外部调用方
    private Boolean accepted = Boolean.FALSE;

    @Column(name = "best_score", nullable = false)
    // 内部实现细节，避免直接暴露给外部调用方
    private Integer bestScore = 0;

    @Column(name = "last_score")
    // 内部实现细节，避免直接暴露给外部调用方
    private Integer lastScore;

    @Column(name = "max_score")
    // 内部实现细节，避免直接暴露给外部调用方
    private Integer maxScore;

    @Column(name = "last_submitted_at")
    // 内部实现细节，避免直接暴露给外部调用方
    private LocalDateTime lastSubmittedAt;

    @Column(name = "accepted_at")
    // 内部实现细节，避免直接暴露给外部调用方
    private LocalDateTime acceptedAt;

    @Column(name = "created_at", nullable = false)
    // 内部实现细节，避免直接暴露给外部调用方
    private LocalDateTime createdAt;

    @Column(name = "updated_at", nullable = false)
    // 内部实现细节，避免直接暴露给外部调用方
    private LocalDateTime updatedAt;

    @PrePersist
    // 对外暴露的方法或字段，通常承接模块间协作
    public void onCreate() {
        LocalDateTime now = LocalDateTime.now();
        this.createdAt = now;
        this.updatedAt = now;
    }

    @PreUpdate
    // 对外暴露的方法或字段，通常承接模块间协作
    public void onUpdate() {
        this.updatedAt = LocalDateTime.now();
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public Long getId() {
        // 返回本阶段计算结果，供上层流程继续使用
        return id;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setId(Long id) {
        this.id = id;
    }

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
    public ProblemEntity getProblem() {
        // 返回本阶段计算结果，供上层流程继续使用
        return problem;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setProblem(ProblemEntity problem) {
        this.problem = problem;
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

    // 对外暴露的方法或字段，通常承接模块间协作
    public LocalDateTime getCreatedAt() {
        // 返回本阶段计算结果，供上层流程继续使用
        return createdAt;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setCreatedAt(LocalDateTime createdAt) {
        this.createdAt = createdAt;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public LocalDateTime getUpdatedAt() {
        // 返回本阶段计算结果，供上层流程继续使用
        return updatedAt;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setUpdatedAt(LocalDateTime updatedAt) {
        this.updatedAt = updatedAt;
    }
}
