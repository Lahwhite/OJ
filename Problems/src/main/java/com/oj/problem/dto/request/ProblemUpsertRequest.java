// 题目模块：该文件负责具体的数据结构、接口或业务逻辑
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

// 对外暴露的方法或字段，通常承接模块间协作
public class ProblemUpsertRequest {

    @NotBlank(message = "题目标题不能为空")
    @Size(max = 255, message = "题目标题长度不能超过255")
    // 内部实现细节，避免直接暴露给外部调用方
    private String title;

    // 题面主要内容
    @NotBlank(message = "题目描述不能为空")
    private String description;

    @NotNull(message = "题目难度不能为空")
    // 内部实现细节，避免直接暴露给外部调用方
    private Difficulty difficulty;

    @Min(value = 1, message = "时间限制必须大于0")
    // 内部实现细节，避免直接暴露给外部调用方
    private Integer timeLimit = 1000;

    @Min(value = 1, message = "内存限制必须大于0")
    // 内部实现细节，避免直接暴露给外部调用方
    private Integer memoryLimit = 128;

    @NotBlank(message = "输入说明不能为空")
    // 内部实现细节，避免直接暴露给外部调用方
    private String inputDescription;

    @NotBlank(message = "输出说明不能为空")
    // 内部实现细节，避免直接暴露给外部调用方
    private String outputDescription;

    @NotBlank(message = "样例输入不能为空")
    // 内部实现细节，避免直接暴露给外部调用方
    private String sampleInput;

    @NotBlank(message = "样例输出不能为空")
    // 内部实现细节，避免直接暴露给外部调用方
    private String sampleOutput;

    // 内部实现细节，避免直接暴露给外部调用方
    private List<@NotBlank(message = "标签不能为空") String> tags = new ArrayList<>();

    @NotEmpty(message = "测试用例不能为空")
    @Valid
    // 内部实现细节，避免直接暴露给外部调用方
    private List<TestCaseRequest> testCases = new ArrayList<>();

    // 内部实现细节，避免直接暴露给外部调用方
    private Boolean isPublic = Boolean.TRUE;

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
    public Difficulty getDifficulty() {
        // 返回本阶段计算结果，供上层流程继续使用
        return difficulty;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setDifficulty(Difficulty difficulty) {
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
    public List<TestCaseRequest> getTestCases() {
        // 返回本阶段计算结果，供上层流程继续使用
        return testCases;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setTestCases(List<TestCaseRequest> testCases) {
        this.testCases = testCases;
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
}
