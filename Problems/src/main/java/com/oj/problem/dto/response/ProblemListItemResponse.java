package com.oj.problem.dto.response;

import java.util.List;

public class ProblemListItemResponse {

    // 列表页里每一行题目的展示数据
    private Long id;
    // 题目标题会展示在列表和详情页，写入前由请求层完成非空校验。
    private String title;
    // 题目难度枚举，前后端统一使用 easy、medium、hard 三档。
    private String difficulty;
    // 题目标签名称集合，服务层会去重并同步标签统计。
    private List<String> tags;
    // 通过率按 acceptedCount / submissionCount 计算，无提交时返回 0。
    private Double passRate;
    // 提交次数统计，由评测回写流程维护。
    private Integer submissionCount;
    // 通过次数统计，仅在 AC 结果回写时递增。
    private Integer acceptedCount;

    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    public String getTitle() {
        return title;
    }

    public void setTitle(String title) {
        this.title = title;
    }

    public String getDifficulty() {
        return difficulty;
    }

    public void setDifficulty(String difficulty) {
        this.difficulty = difficulty;
    }

    public List<String> getTags() {
        return tags;
    }

    public void setTags(List<String> tags) {
        this.tags = tags;
    }

    public Double getPassRate() {
        return passRate;
    }

    public void setPassRate(Double passRate) {
        this.passRate = passRate;
    }

    public Integer getSubmissionCount() {
        return submissionCount;
    }

    public void setSubmissionCount(Integer submissionCount) {
        this.submissionCount = submissionCount;
    }

    public Integer getAcceptedCount() {
        return acceptedCount;
    }

    public void setAcceptedCount(Integer acceptedCount) {
        this.acceptedCount = acceptedCount;
    }
}
