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
public class ProblemUserStatusEntity {

    // 数据库主键，所有对外响应都沿用这个标识定位资源。
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    // 这里只记用户 id，不再额外连用户表
    @Column(name = "user_id", nullable = false)
    private Long userId;

    // 关联的题目实体，状态和测试用例都依附于题目生命周期。
    @ManyToOne(fetch = FetchType.LAZY, optional = false)
    @JoinColumn(name = "problem_id", nullable = false)
    private ProblemEntity problem;

    // 是否已经通过该题，历史上任意一次 AC 后保持为 true。
    @Column(nullable = false)
    private Boolean accepted = Boolean.FALSE;

    // 历史最高分，用于部分通过的题目展示用户最好成绩。
    @Column(name = "best_score", nullable = false)
    private Integer bestScore = 0;

    // 最近一次提交得分，便于用户判断当前进展。
    @Column(name = "last_score")
    private Integer lastScore;

    // 本题当前评测总分，和 score 一起展示分数占比。
    @Column(name = "max_score")
    private Integer maxScore;

    // 最近一次提交时间，用于题目列表和状态详情展示。
    @Column(name = "last_submitted_at")
    private LocalDateTime lastSubmittedAt;

    // 首次通过时间，只有从未通过变为通过时写入。
    @Column(name = "accepted_at")
    private LocalDateTime acceptedAt;

    // 创建时间由数据库自动维护，用于列表排序和审计展示。
    @Column(name = "created_at", nullable = false)
    private LocalDateTime createdAt;

    // 更新时间由数据库自动维护，用于管理端确认最近修改。
    @Column(name = "updated_at", nullable = false)
    private LocalDateTime updatedAt;

    @PrePersist
    public void onCreate() {
        LocalDateTime now = LocalDateTime.now();
        this.createdAt = now;
        this.updatedAt = now;
    }

    @PreUpdate
    public void onUpdate() {
        this.updatedAt = LocalDateTime.now();
    }

    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    public Long getUserId() {
        return userId;
    }

    public void setUserId(Long userId) {
        this.userId = userId;
    }

    public ProblemEntity getProblem() {
        return problem;
    }

    public void setProblem(ProblemEntity problem) {
        this.problem = problem;
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

    public LocalDateTime getCreatedAt() {
        return createdAt;
    }

    public void setCreatedAt(LocalDateTime createdAt) {
        this.createdAt = createdAt;
    }

    public LocalDateTime getUpdatedAt() {
        return updatedAt;
    }

    public void setUpdatedAt(LocalDateTime updatedAt) {
        this.updatedAt = updatedAt;
    }
}
