// 题目模块：该文件负责具体的数据结构、接口或业务逻辑
package com.oj.problem.entity;

import java.time.LocalDateTime;
import java.util.ArrayList;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;
import javax.persistence.CascadeType;
import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.EnumType;
import javax.persistence.Enumerated;
import javax.persistence.FetchType;
import javax.persistence.GeneratedValue;
import javax.persistence.GenerationType;
import javax.persistence.Id;
import javax.persistence.JoinColumn;
import javax.persistence.JoinTable;
import javax.persistence.ManyToMany;
import javax.persistence.OneToMany;
import javax.persistence.PrePersist;
import javax.persistence.PreUpdate;
import javax.persistence.Table;

@Entity
@Table(name = "problems")
// 对外暴露的方法或字段，通常承接模块间协作
public class ProblemEntity {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    // 内部实现细节，避免直接暴露给外部调用方
    private Long id;

    // 题面基础信息
    @Column(nullable = false, length = 255)
    private String title;

    @Column(nullable = false, columnDefinition = "TEXT")
    // 内部实现细节，避免直接暴露给外部调用方
    private String description;

    @Enumerated(EnumType.STRING)
    @Column(nullable = false, length = 20)
    // 内部实现细节，避免直接暴露给外部调用方
    private Difficulty difficulty;

    @Column(name = "time_limit", nullable = false)
    // 内部实现细节，避免直接暴露给外部调用方
    private Integer timeLimit = 1000;

    @Column(name = "memory_limit", nullable = false)
    // 内部实现细节，避免直接暴露给外部调用方
    private Integer memoryLimit = 128;

    @Column(name = "input_description", nullable = false, columnDefinition = "TEXT")
    // 内部实现细节，避免直接暴露给外部调用方
    private String inputDescription;

    @Column(name = "output_description", nullable = false, columnDefinition = "TEXT")
    // 内部实现细节，避免直接暴露给外部调用方
    private String outputDescription;

    @Column(name = "sample_input", nullable = false, columnDefinition = "TEXT")
    // 内部实现细节，避免直接暴露给外部调用方
    private String sampleInput;

    @Column(name = "sample_output", nullable = false, columnDefinition = "TEXT")
    // 内部实现细节，避免直接暴露给外部调用方
    private String sampleOutput;

    @Column(name = "created_by")
    // 内部实现细节，避免直接暴露给外部调用方
    private Long createdBy;

    @Column(name = "is_public", nullable = false)
    // 内部实现细节，避免直接暴露给外部调用方
    private Boolean isPublic = Boolean.TRUE;

    @Column(name = "submission_count", nullable = false)
    // 内部实现细节，避免直接暴露给外部调用方
    private Integer submissionCount = 0;

    @Column(name = "accepted_count", nullable = false)
    // 内部实现细节，避免直接暴露给外部调用方
    private Integer acceptedCount = 0;

    @Column(name = "created_at", nullable = false)
    // 内部实现细节，避免直接暴露给外部调用方
    private LocalDateTime createdAt;

    @Column(name = "updated_at", nullable = false)
    // 内部实现细节，避免直接暴露给外部调用方
    private LocalDateTime updatedAt;

    @OneToMany(mappedBy = "problem", cascade = CascadeType.ALL, orphanRemoval = true)
    // 内部实现细节，避免直接暴露给外部调用方
    private List<TestCaseEntity> testCases = new ArrayList<>();

    @ManyToMany(fetch = FetchType.LAZY, cascade = {CascadeType.PERSIST, CascadeType.MERGE})
    @JoinTable(
            name = "problem_tags",
            joinColumns = @JoinColumn(name = "problem_id"),
            inverseJoinColumns = @JoinColumn(name = "tag_id"))
    // 内部实现细节，避免直接暴露给外部调用方
    private Set<TagEntity> tags = new LinkedHashSet<>();

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
    public void addTestCase(TestCaseEntity testCase) {
        testCases.add(testCase);
        testCase.setProblem(this);
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void clearTestCases() {
        // 循环处理：逐项遍历集合并完成同步或映射
        for (TestCaseEntity testCase : testCases) {
            testCase.setProblem(null);
        }
        testCases.clear();
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
    public String getTitle() {
        // 返回本阶段计算结果，供上层流程继续使用
        return title;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setTitle(String title) {
        this.title = title;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public String getDescription() {
        // 返回本阶段计算结果，供上层流程继续使用
        return description;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setDescription(String description) {
        this.description = description;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public Difficulty getDifficulty() {
        // 返回本阶段计算结果，供上层流程继续使用
        return difficulty;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setDifficulty(Difficulty difficulty) {
        this.difficulty = difficulty;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public Integer getTimeLimit() {
        // 返回本阶段计算结果，供上层流程继续使用
        return timeLimit;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setTimeLimit(Integer timeLimit) {
        this.timeLimit = timeLimit;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public Integer getMemoryLimit() {
        // 返回本阶段计算结果，供上层流程继续使用
        return memoryLimit;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setMemoryLimit(Integer memoryLimit) {
        this.memoryLimit = memoryLimit;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public String getInputDescription() {
        // 返回本阶段计算结果，供上层流程继续使用
        return inputDescription;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setInputDescription(String inputDescription) {
        this.inputDescription = inputDescription;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public String getOutputDescription() {
        // 返回本阶段计算结果，供上层流程继续使用
        return outputDescription;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setOutputDescription(String outputDescription) {
        this.outputDescription = outputDescription;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public String getSampleInput() {
        // 返回本阶段计算结果，供上层流程继续使用
        return sampleInput;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setSampleInput(String sampleInput) {
        this.sampleInput = sampleInput;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public String getSampleOutput() {
        // 返回本阶段计算结果，供上层流程继续使用
        return sampleOutput;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setSampleOutput(String sampleOutput) {
        this.sampleOutput = sampleOutput;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public Long getCreatedBy() {
        // 返回本阶段计算结果，供上层流程继续使用
        return createdBy;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setCreatedBy(Long createdBy) {
        this.createdBy = createdBy;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public Boolean getIsPublic() {
        // 返回本阶段计算结果，供上层流程继续使用
        return isPublic;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setIsPublic(Boolean isPublic) {
        this.isPublic = isPublic;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public Integer getSubmissionCount() {
        // 返回本阶段计算结果，供上层流程继续使用
        return submissionCount;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setSubmissionCount(Integer submissionCount) {
        this.submissionCount = submissionCount;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public Integer getAcceptedCount() {
        // 返回本阶段计算结果，供上层流程继续使用
        return acceptedCount;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setAcceptedCount(Integer acceptedCount) {
        this.acceptedCount = acceptedCount;
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

    // 对外暴露的方法或字段，通常承接模块间协作
    public List<TestCaseEntity> getTestCases() {
        // 返回本阶段计算结果，供上层流程继续使用
        return testCases;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setTestCases(List<TestCaseEntity> testCases) {
        this.testCases = testCases;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public Set<TagEntity> getTags() {
        // 返回本阶段计算结果，供上层流程继续使用
        return tags;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setTags(Set<TagEntity> tags) {
        this.tags = tags;
    }
}
