package com.oj.problem.dto.request;

import javax.validation.constraints.Min;
import javax.validation.constraints.NotBlank;
import javax.validation.constraints.NotNull;

public class TestCaseRequest {

    // 创建或更新题目时带上的测试用例
    @NotBlank(message = "测试输入不能为空")
    private String input;

    // 测试用例期望输出，评测前会写入 expect json。
    @NotBlank(message = "测试输出不能为空")
    private String output;

    // 是否公开为样例，只影响前端展示，不影响评测用例生成。
    @NotNull(message = "是否样例不能为空")
    private Boolean isSample;

    // 单个测试点分值，评测结果会累加为 total_score。
    @Min(value = 0, message = "分值不能小于0")
    private Integer score = 0;

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
}
