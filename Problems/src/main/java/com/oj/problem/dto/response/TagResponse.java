package com.oj.problem.dto.response;

public class TagResponse {

    // 标签管理和筛选展示都复用这个响应对象
    private Long id;
    private String name;
    // 标签颜色预留字段，目前主要用于前端扩展展示。
    private String color;
    // 使用该标签的题目数量，同步标签时增减。
    private Integer problemCount;

    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getColor() {
        return color;
    }

    public void setColor(String color) {
        this.color = color;
    }

    public Integer getProblemCount() {
        return problemCount;
    }

    public void setProblemCount(Integer problemCount) {
        this.problemCount = problemCount;
    }
}
