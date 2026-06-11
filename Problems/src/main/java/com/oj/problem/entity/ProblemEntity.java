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
public class ProblemEntity {

    // 数据库主键，所有对外响应都沿用这个标识定位资源。
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    // 题面基础信息
    @Column(nullable = false, length = 255)
    private String title;

    // 题面正文，允许多行文本并在前端按原换行展示。
    @Column(nullable = false, columnDefinition = "TEXT")
    private String description;

    // 题目难度枚举，前后端统一使用 easy、medium、hard 三档。
    @Enumerated(EnumType.STRING)
    @Column(nullable = false, length = 20)
    private Difficulty difficulty;

    // 单个测试点的时间限制，单位为毫秒。
    @Column(name = "time_limit", nullable = false)
    private Integer timeLimit = 1000;

    // 单个测试点的内存限制，单位为 MB。
    @Column(name = "memory_limit", nullable = false)
    private Integer memoryLimit = 128;

    // 输入格式说明，和样例输入分开保存便于页面分区渲染。
    @Column(name = "input_description", nullable = false, columnDefinition = "TEXT")
    private String inputDescription;

    // 输出格式说明，和样例输出分开保存便于页面分区渲染。
    @Column(name = "output_description", nullable = false, columnDefinition = "TEXT")
    private String outputDescription;

    // 公开样例输入，用于详情页展示和用户本地调试参考。
    @Column(name = "sample_input", nullable = false, columnDefinition = "TEXT")
    private String sampleInput;

    // 公开样例输出，用于详情页展示和用户本地调试参考。
    @Column(name = "sample_output", nullable = false, columnDefinition = "TEXT")
    private String sampleOutput;

    // 创建者用户 ID，首次保存题目时从当前登录用户写入。
    @Column(name = "created_by")
    private Long createdBy;

    // 是否公开给普通用户，管理端可以保存未公开题目草稿。
    @Column(name = "is_public", nullable = false)
    private Boolean isPublic = Boolean.TRUE;

    // 提交次数统计，由评测回写流程维护。
    @Column(name = "submission_count", nullable = false)
    private Integer submissionCount = 0;

    // 通过次数统计，仅在 AC 结果回写时递增。
    @Column(name = "accepted_count", nullable = false)
    private Integer acceptedCount = 0;

    // 创建时间由数据库自动维护，用于列表排序和审计展示。
    @Column(name = "created_at", nullable = false)
    private LocalDateTime createdAt;

    // 更新时间由数据库自动维护，用于管理端确认最近修改。
    @Column(name = "updated_at", nullable = false)
    private LocalDateTime updatedAt;

    // 完整测试用例集合，创建和更新题目时至少保留一条。
    @OneToMany(mappedBy = "problem", cascade = CascadeType.ALL, orphanRemoval = true)
    private List<TestCaseEntity> testCases = new ArrayList<>();

    @ManyToMany(fetch = FetchType.LAZY, cascade = {CascadeType.PERSIST, CascadeType.MERGE})
    @JoinTable(
            name = "problem_tags",
            joinColumns = @JoinColumn(name = "problem_id"),
            inverseJoinColumns = @JoinColumn(name = "tag_id"))
    // 题目标签名称集合，服务层会去重并同步标签统计。
    private Set<TagEntity> tags = new LinkedHashSet<>();

    @PrePersist
    public void onCreate() {
        LocalDateTime now = LocalDateTime.now();
        this.createdAt = now;
        this.updatedAt = now;
    }

    @PreUpdate
    public void onUpdate() {
        this.updatedAt = LocalDateTime.now();
    }

    public void addTestCase(TestCaseEntity testCase) {
        testCases.add(testCase);
        testCase.setProblem(this);
    }

    public void clearTestCases() {
        for (TestCaseEntity testCase : testCases) {
            testCase.setProblem(null);
        }
        testCases.clear();
    }

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

    public Difficulty getDifficulty() {
        return difficulty;
    }

    public void setDifficulty(Difficulty difficulty) {
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

    public Long getCreatedBy() {
        return createdBy;
    }

    public void setCreatedBy(Long createdBy) {
        this.createdBy = createdBy;
    }

    public Boolean getIsPublic() {
        return isPublic;
    }

    public void setIsPublic(Boolean isPublic) {
        this.isPublic = isPublic;
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

    public List<TestCaseEntity> getTestCases() {
        return testCases;
    }

    public void setTestCases(List<TestCaseEntity> testCases) {
        this.testCases = testCases;
    }

    public Set<TagEntity> getTags() {
        return tags;
    }

    public void setTags(Set<TagEntity> tags) {
        this.tags = tags;
    }
}
