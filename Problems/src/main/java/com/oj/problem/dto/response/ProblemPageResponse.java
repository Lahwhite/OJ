package com.oj.problem.dto.response;

import java.util.List;

public class ProblemPageResponse {

    // 分页查询的统一返回结构
    private long total;
    // 页码从 1 开始，服务层转换为 PageRequest 时会减 1。
    private int page;
    // 每页数量在请求对象中限制上限，避免一次查询过多数据。
    private int size;
    // 当前页题目摘要列表，只包含列表渲染需要的字段。
    private List<ProblemListItemResponse> problems;

    public long getTotal() {
        return total;
    }

    public void setTotal(long total) {
        this.total = total;
    }

    public int getPage() {
        return page;
    }

    public void setPage(int page) {
        this.page = page;
    }

    public int getSize() {
        return size;
    }

    public void setSize(int size) {
        this.size = size;
    }

    public List<ProblemListItemResponse> getProblems() {
        return problems;
    }

    public void setProblems(List<ProblemListItemResponse> problems) {
        this.problems = problems;
    }
}
