package com.oj.problem.dto.request;

import com.oj.problem.entity.Difficulty;
import java.util.ArrayList;
import java.util.List;
import javax.validation.Valid;
import javax.validation.constraints.Min;
import javax.validation.constraints.NotBlank;
import javax.validation.constraints.NotEmpty;
import javax.validation.constraints.NotNull;
import javax.validation.constraints.Size;

public class ProblemUpsertRequest {

    // 题目标题会展示在列表和详情页，写入前由请求层完成非空校验。
    @NotBlank(message = "题目标题不能为空")
    @Size(max = 255, message = "题目标题长度不能超过255")
    private String title;

    // 题面主要内容
    @NotBlank(message = "题目描述不能为空")
    private String description;

    // 题目难度枚举，前后端统一使用 easy、medium、hard 三档。
    @NotNull(message = "题目难度不能为空")
    private Difficulty difficulty;

    // 单个测试点的时间限制，单位为毫秒。
    @Min(value = 1, message = "时间限制必须大于0")
    private Integer timeLimit = 1000;

    // 单个测试点的内存限制，单位为 MB。
    @Min(value = 1, message = "内存限制必须大于0")
    private Integer memoryLimit = 128;

    // 输入格式说明，和样例输入分开保存便于页面分区渲染。
    @NotBlank(message = "输入说明不能为空")
    private String inputDescription;

    // 输出格式说明，和样例输出分开保存便于页面分区渲染。
    @NotBlank(message = "输出说明不能为空")
    private String outputDescription;

    // 公开样例输入，用于详情页展示和用户本地调试参考。
    @NotBlank(message = "样例输入不能为空")
    private String sampleInput;

    // 公开样例输出，用于详情页展示和用户本地调试参考。
    @NotBlank(message = "样例输出不能为空")
    private String sampleOutput;

    private List<@NotBlank(message = "标签不能为空") String> tags = new ArrayList<>();

    // 完整测试用例集合，创建和更新题目时至少保留一条。
    @NotEmpty(message = "测试用例不能为空")
    @Valid
    private List<TestCaseRequest> testCases = new ArrayList<>();

    // 是否公开给普通用户，管理端可以保存未公开题目草稿。
    private Boolean isPublic = Boolean.TRUE;

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

    public List<String> getTags() {
        return tags;
    }

    public void setTags(List<String> tags) {
        this.tags = tags;
    }

    public List<TestCaseRequest> getTestCases() {
        return testCases;
    }

    public void setTestCases(List<TestCaseRequest> testCases) {
        this.testCases = testCases;
    }

    public Boolean getIsPublic() {
        return isPublic;
    }

    public void setIsPublic(Boolean isPublic) {
        this.isPublic = isPublic;
    }
}
