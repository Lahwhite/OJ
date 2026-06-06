// 题目模块：该文件负责具体的数据结构、接口或业务逻辑
package com.oj.problem.dto.response;

import java.util.List;

// 对外暴露的方法或字段，通常承接模块间协作
public class ProblemPageResponse {

    // 分页查询的统一返回结构
    private long total;
    private int page;
    // 内部实现细节，避免直接暴露给外部调用方
    private int size;
    // 内部实现细节，避免直接暴露给外部调用方
    private List<ProblemListItemResponse> problems;

    // 对外暴露的方法或字段，通常承接模块间协作
    public long getTotal() {
        // 返回本阶段计算结果，供上层流程继续使用
        return total;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setTotal(long total) {
        this.total = total;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public int getPage() {
        // 返回本阶段计算结果，供上层流程继续使用
        return page;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setPage(int page) {
        this.page = page;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public int getSize() {
        // 返回本阶段计算结果，供上层流程继续使用
        return size;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setSize(int size) {
        this.size = size;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public List<ProblemListItemResponse> getProblems() {
        // 返回本阶段计算结果，供上层流程继续使用
        return problems;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setProblems(List<ProblemListItemResponse> problems) {
        this.problems = problems;
    }
}
