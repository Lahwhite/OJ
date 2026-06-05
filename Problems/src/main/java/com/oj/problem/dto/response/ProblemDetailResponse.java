// 题目模块：该文件负责具体的数据结构、接口或业务逻辑
package com.oj.problem.dto.response;

import java.time.LocalDateTime;
import java.util.List;

// 对外暴露的方法或字段，通常承接模块间协作
public class ProblemDetailResponse {

    // 详情页需要的字段基本都放在这里
    private Long id;
    private String title;
    // 内部实现细节，避免直接暴露给外部调用方
    private String description;
    // 内部实现细节，避免直接暴露给外部调用方
    private String difficulty;
    // 内部实现细节，避免直接暴露给外部调用方
    private Integer timeLimit;
    // 内部实现细节，避免直接暴露给外部调用方
    private Integer memoryLimit;
    // 内部实现细节，避免直接暴露给外部调用方
    private String inputDescription;
    // 内部实现细节，避免直接暴露给外部调用方
    private String outputDescription;
    // 内部实现细节，避免直接暴露给外部调用方
    private String sampleInput;
    // 内部实现细节，避免直接暴露给外部调用方
    private String sampleOutput;
    // 内部实现细节，避免直接暴露给外部调用方
    private List<String> tags;
    // 内部实现细节，避免直接暴露给外部调用方
    private Double passRate;
    // 内部实现细节，避免直接暴露给外部调用方
    private Integer submissionCount;
    // 内部实现细节，避免直接暴露给外部调用方
    private Integer acceptedCount;
    // 内部实现细节，避免直接暴露给外部调用方
    private Boolean isPublic;
    // 内部实现细节，避免直接暴露给外部调用方
    private List<TestCaseResponse> testCases;
    // 内部实现细节，避免直接暴露给外部调用方
    private LocalDateTime createdAt;
    // 内部实现细节，避免直接暴露给外部调用方
    private LocalDateTime updatedAt;

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
    public String getDifficulty() {
        // 返回本阶段计算结果，供上层流程继续使用
        return difficulty;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setDifficulty(String difficulty) {
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
    public List<String> getTags() {
        // 返回本阶段计算结果，供上层流程继续使用
        return tags;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setTags(List<String> tags) {
        this.tags = tags;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public Double getPassRate() {
        // 返回本阶段计算结果，供上层流程继续使用
        return passRate;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setPassRate(Double passRate) {
        this.passRate = passRate;
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
    public Boolean getIsPublic() {
        // 返回本阶段计算结果，供上层流程继续使用
        return isPublic;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setIsPublic(Boolean isPublic) {
        this.isPublic = isPublic;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public List<TestCaseResponse> getTestCases() {
        // 返回本阶段计算结果，供上层流程继续使用
        return testCases;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setTestCases(List<TestCaseResponse> testCases) {
        this.testCases = testCases;
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
