package com.oj.problem.dto.response;

import java.time.LocalDateTime;
import java.util.List;

public class ProblemDetailResponse {

    // 详情页需要的字段基本都放在这里
    private Long id;
    // 题目标题会展示在列表和详情页，写入前由请求层完成非空校验。
    private String title;
    // 题面正文，允许多行文本并在前端按原换行展示。
    private String description;
    // 题目难度枚举，前后端统一使用 easy、medium、hard 三档。
    private String difficulty;
    // 单个测试点的时间限制，单位为毫秒。
    private Integer timeLimit;
    // 单个测试点的内存限制，单位为 MB。
    private Integer memoryLimit;
    // 输入格式说明，和样例输入分开保存便于页面分区渲染。
    private String inputDescription;
    // 输出格式说明，和样例输出分开保存便于页面分区渲染。
    private String outputDescription;
    // 公开样例输入，用于详情页展示和用户本地调试参考。
    private String sampleInput;
    // 公开样例输出，用于详情页展示和用户本地调试参考。
    private String sampleOutput;
    // 题目标签名称集合，服务层会去重并同步标签统计。
    private List<String> tags;
    // 通过率按 acceptedCount / submissionCount 计算，无提交时返回 0。
    private Double passRate;
    // 提交次数统计，由评测回写流程维护。
    private Integer submissionCount;
    // 通过次数统计，仅在 AC 结果回写时递增。
    private Integer acceptedCount;
    // 是否公开给普通用户，管理端可以保存未公开题目草稿。
    private Boolean isPublic;
    // 完整测试用例集合，创建和更新题目时至少保留一条。
    private List<TestCaseResponse> testCases;
    // 创建时间由数据库自动维护，用于列表排序和审计展示。
    private LocalDateTime createdAt;
    // 更新时间由数据库自动维护，用于管理端确认最近修改。
    private LocalDateTime updatedAt;

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

    public String getDescription() {
        return description;
    }

    public void setDescription(String description) {
        this.description = description;
    }

    public String getDifficulty() {
        return difficulty;
    }

    public void setDifficulty(String difficulty) {
        this.difficulty = difficulty;
    }

    public Integer getTimeLimit() {
        return timeLimit;
    }

    public void setTimeLimit(Integer timeLimit) {
        this.timeLimit = timeLimit;
    }

    public Integer getMemoryLimit() {
        return memoryLimit;
    }

    public void setMemoryLimit(Integer memoryLimit) {
        this.memoryLimit = memoryLimit;
    }

    public String getInputDescription() {
        return inputDescription;
    }

    public void setInputDescription(String inputDescription) {
        this.inputDescription = inputDescription;
    }

    public String getOutputDescription() {
        return outputDescription;
    }

    public void setOutputDescription(String outputDescription) {
        this.outputDescription = outputDescription;
    }

    public String getSampleInput() {
        return sampleInput;
    }

    public void setSampleInput(String sampleInput) {
        this.sampleInput = sampleInput;
    }

    public String getSampleOutput() {
        return sampleOutput;
    }

    public void setSampleOutput(String sampleOutput) {
        this.sampleOutput = sampleOutput;
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

    public Boolean getIsPublic() {
        return isPublic;
    }

    public void setIsPublic(Boolean isPublic) {
        this.isPublic = isPublic;
    }

    public List<TestCaseResponse> getTestCases() {
        return testCases;
    }

    public void setTestCases(List<TestCaseResponse> testCases) {
        this.testCases = testCases;
    }

    public LocalDateTime getCreatedAt() {
        return createdAt;
    }

    public void setCreatedAt(LocalDateTime createdAt) {
        this.createdAt = createdAt;
    }

    public LocalDateTime getUpdatedAt() {
        return updatedAt;
    }

    public void setUpdatedAt(LocalDateTime updatedAt) {
        this.updatedAt = updatedAt;
    }
}
