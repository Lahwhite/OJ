// 题目模块：该文件负责具体的数据结构、接口或业务逻辑
package com.oj.problem.dto.response;

// 对外暴露的方法或字段，通常承接模块间协作
public class TagResponse {

    // 标签管理和筛选展示都复用这个响应对象
    private Long id;
    private String name;
    // 内部实现细节，避免直接暴露给外部调用方
    private String color;
    // 内部实现细节，避免直接暴露给外部调用方
    private Integer problemCount;

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
    public String getName() {
        // 返回本阶段计算结果，供上层流程继续使用
        return name;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setName(String name) {
        this.name = name;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public String getColor() {
        // 返回本阶段计算结果，供上层流程继续使用
        return color;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setColor(String color) {
        this.color = color;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public Integer getProblemCount() {
        // 返回本阶段计算结果，供上层流程继续使用
        return problemCount;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setProblemCount(Integer problemCount) {
        this.problemCount = problemCount;
    }
}
