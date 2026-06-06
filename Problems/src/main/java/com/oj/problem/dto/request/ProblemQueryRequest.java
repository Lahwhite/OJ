// 题目模块：该文件负责具体的数据结构、接口或业务逻辑
package com.oj.problem.dto.request;

import com.oj.problem.entity.Difficulty;
import javax.validation.constraints.Max;
import javax.validation.constraints.Min;

// 对外暴露的方法或字段，通常承接模块间协作
public class ProblemQueryRequest {

    @Min(1)
    // 内部实现细节，避免直接暴露给外部调用方
    private Integer page = 1;

    // 每页默认 20 条，最多给到 100，防止一次查太多
    @Max(100)
    private Integer size = 20;

    // 内部实现细节，避免直接暴露给外部调用方
    private Difficulty difficulty;

    // 内部实现细节，避免直接暴露给外部调用方
    private String tag;

    // 内部实现细节，避免直接暴露给外部调用方
    private String keyword;

    // 对外暴露的方法或字段，通常承接模块间协作
    public Integer getPage() {
        // 返回本阶段计算结果，供上层流程继续使用
        return page;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setPage(Integer page) {
        this.page = page;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public Integer getSize() {
        // 返回本阶段计算结果，供上层流程继续使用
        return size;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setSize(Integer size) {
        this.size = size;
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
    public String getTag() {
        // 返回本阶段计算结果，供上层流程继续使用
        return tag;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setTag(String tag) {
        this.tag = tag;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public String getKeyword() {
        // 返回本阶段计算结果，供上层流程继续使用
        return keyword;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setKeyword(String keyword) {
        this.keyword = keyword;
    }
}
