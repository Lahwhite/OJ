package com.oj.problem.entity;

import java.time.LocalDateTime;
import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.FetchType;
import javax.persistence.GeneratedValue;
import javax.persistence.GenerationType;
import javax.persistence.Id;
import javax.persistence.JoinColumn;
import javax.persistence.ManyToOne;
import javax.persistence.PrePersist;
import javax.persistence.Table;

@Entity
@Table(name = "test_cases")
public class TestCaseEntity {

    // 数据库主键，所有对外响应都沿用这个标识定位资源。
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    // 一个题目对应多个测试用例
    @ManyToOne(fetch = FetchType.LAZY)
    @JoinColumn(name = "problem_id", nullable = false)
    private ProblemEntity problem;

    // 测试用例输入内容，保存原始文本以兼容多行数据。
    @Column(nullable = false, columnDefinition = "TEXT")
    private String input;

    // 测试用例期望输出，评测前会写入 expect json。
    @Column(nullable = false, columnDefinition = "TEXT")
    private String output;

    // 是否公开为样例，只影响前端展示，不影响评测用例生成。
    @Column(name = "is_sample", nullable = false)
    private Boolean isSample = Boolean.FALSE;

    // 单个测试点分值，评测结果会累加为 total_score。
    @Column(nullable = false)
    private Integer score = 0;

    // 创建时间由数据库自动维护，用于列表排序和审计展示。
    @Column(name = "created_at", nullable = false)
    private LocalDateTime createdAt;

    @PrePersist
    public void onCreate() {
        this.createdAt = LocalDateTime.now();
    }

    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    public ProblemEntity getProblem() {
        return problem;
    }

    public void setProblem(ProblemEntity problem) {
        this.problem = problem;
    }

    public String getInput() {
        return input;
    }

    public void setInput(String input) {
        this.input = input;
    }

    public String getOutput() {
        return output;
    }

    public void setOutput(String output) {
        this.output = output;
    }

    public Boolean getIsSample() {
        return isSample;
    }

    public void setIsSample(Boolean isSample) {
        this.isSample = isSample;
    }

    public Integer getScore() {
        return score;
    }

    public void setScore(Integer score) {
        this.score = score;
    }

    public LocalDateTime getCreatedAt() {
        return createdAt;
    }

    public void setCreatedAt(LocalDateTime createdAt) {
        this.createdAt = createdAt;
    }
}
