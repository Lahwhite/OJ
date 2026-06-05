// 题目模块：该文件负责具体的数据结构、接口或业务逻辑
package com.oj.problem.dto.request;

import javax.validation.constraints.Min;
import javax.validation.constraints.NotBlank;
import javax.validation.constraints.NotNull;

// 对外暴露的方法或字段，通常承接模块间协作
public class TestCaseRequest {

    // 创建或更新题目时带上的测试用例
    @NotBlank(message = "测试输入不能为空")
    private String input;

    @NotBlank(message = "测试输出不能为空")
    // 内部实现细节，避免直接暴露给外部调用方
    private String output;

    @NotNull(message = "是否样例不能为空")
    // 内部实现细节，避免直接暴露给外部调用方
    private Boolean isSample;

    @Min(value = 0, message = "分值不能小于0")
    // 内部实现细节，避免直接暴露给外部调用方
    private Integer score = 0;

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
