package com.oj.problem.dto.request;

import com.oj.problem.entity.Difficulty;
import javax.validation.constraints.Max;
import javax.validation.constraints.Min;

public class ProblemQueryRequest {

    // 页码从 1 开始，服务层转换为 PageRequest 时会减 1。
    @Min(1)
    private Integer page = 1;

    // 每页默认 20 条，最多给到 100，防止一次查太多
    @Max(100)
    private Integer size = 20;

    // 题目难度枚举，前后端统一使用 easy、medium、hard 三档。
    private Difficulty difficulty;

    // 按单个标签过滤列表时使用，服务层会做大小写归一。
    private String tag;

    // 标题和题面关键字，服务层使用 like 查询进行模糊匹配。
    private String keyword;

    public Integer getPage() {
        return page;
    }

    public void setPage(Integer page) {
        this.page = page;
    }

    public Integer getSize() {
        return size;
    }

    public void setSize(Integer size) {
        this.size = size;
    }

    public Difficulty getDifficulty() {
        return difficulty;
    }

    public void setDifficulty(Difficulty difficulty) {
        this.difficulty = difficulty;
    }

    public String getTag() {
        return tag;
    }

    public void setTag(String tag) {
        this.tag = tag;
    }

    public String getKeyword() {
        return keyword;
    }

    public void setKeyword(String keyword) {
        this.keyword = keyword;
    }
}
