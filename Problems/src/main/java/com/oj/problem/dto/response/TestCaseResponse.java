package com.oj.problem.dto.response;

public class TestCaseResponse {

    // 这里只返回测试用例的基础信息
    private Long id;
    // 测试用例输入内容，保存原始文本以兼容多行数据。
    private String input;
    // 测试用例期望输出，评测前会写入 expect json。
    private String output;
    // 是否公开为样例，只影响前端展示，不影响评测用例生成。
    private Boolean isSample;
    // 单个测试点分值，评测结果会累加为 total_score。
    private Integer score;

    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
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
}
