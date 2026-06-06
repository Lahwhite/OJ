// 题目模块：该文件负责具体的数据结构、接口或业务逻辑
package com.oj.problem.dto.response;

import java.util.List;

// 对外暴露的方法或字段，通常承接模块间协作
public class ProblemListItemResponse {

    // 列表页里每一行题目的展示数据
    private Long id;
    private String title;
    // 内部实现细节，避免直接暴露给外部调用方
    private String difficulty;
    // 内部实现细节，避免直接暴露给外部调用方
    private List<String> tags;
    // 内部实现细节，避免直接暴露给外部调用方
    private Double passRate;
    // 内部实现细节，避免直接暴露给外部调用方
    private Integer submissionCount;
    // 内部实现细节，避免直接暴露给外部调用方
    private Integer acceptedCount;

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
    public String getDifficulty() {
        // 返回本阶段计算结果，供上层流程继续使用
        return difficulty;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setDifficulty(String difficulty) {
        this.difficulty = difficulty;
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
}
