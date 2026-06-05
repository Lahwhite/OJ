// 题目模块：该文件负责具体的数据结构、接口或业务逻辑
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
// 对外暴露的方法或字段，通常承接模块间协作
public class TestCaseEntity {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    // 内部实现细节，避免直接暴露给外部调用方
    private Long id;

    // 一个题目对应多个测试用例
    @ManyToOne(fetch = FetchType.LAZY)
    @JoinColumn(name = "problem_id", nullable = false)
    // 内部实现细节，避免直接暴露给外部调用方
    private ProblemEntity problem;

    @Column(nullable = false, columnDefinition = "TEXT")
    // 内部实现细节，避免直接暴露给外部调用方
    private String input;

    @Column(nullable = false, columnDefinition = "TEXT")
    // 内部实现细节，避免直接暴露给外部调用方
    private String output;

    @Column(name = "is_sample", nullable = false)
    // 内部实现细节，避免直接暴露给外部调用方
    private Boolean isSample = Boolean.FALSE;

    @Column(nullable = false)
    // 内部实现细节，避免直接暴露给外部调用方
    private Integer score = 0;

    @Column(name = "created_at", nullable = false)
    // 内部实现细节，避免直接暴露给外部调用方
    private LocalDateTime createdAt;

    @PrePersist
    // 对外暴露的方法或字段，通常承接模块间协作
    public void onCreate() {
        this.createdAt = LocalDateTime.now();
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
    public ProblemEntity getProblem() {
        // 返回本阶段计算结果，供上层流程继续使用
        return problem;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setProblem(ProblemEntity problem) {
        this.problem = problem;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public String getInput() {
        // 返回本阶段计算结果，供上层流程继续使用
        return input;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setInput(String input) {
        this.input = input;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public String getOutput() {
        // 返回本阶段计算结果，供上层流程继续使用
        return output;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setOutput(String output) {
        this.output = output;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public Boolean getIsSample() {
        // 返回本阶段计算结果，供上层流程继续使用
        return isSample;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setIsSample(Boolean isSample) {
        this.isSample = isSample;
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
    public LocalDateTime getCreatedAt() {
        // 返回本阶段计算结果，供上层流程继续使用
        return createdAt;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setCreatedAt(LocalDateTime createdAt) {
        this.createdAt = createdAt;
    }
}
