// 题目模块：该文件负责具体的数据结构、接口或业务逻辑
package com.oj.problem.dto.response;

// 对外暴露的方法或字段，通常承接模块间协作
public class TestCaseResponse {

    // 这里只返回测试用例的基础信息
    private Long id;
    private String input;
    // 内部实现细节，避免直接暴露给外部调用方
    private String output;
    // 内部实现细节，避免直接暴露给外部调用方
    private Boolean isSample;
    // 内部实现细节，避免直接暴露给外部调用方
    private Integer score;

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
}
