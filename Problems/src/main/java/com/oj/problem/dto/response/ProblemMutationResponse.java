package com.oj.problem.dto.response;

import java.time.LocalDateTime;

public class ProblemMutationResponse {

    // 创建或更新题目后，前端回显用这几个字段就够了
    private Long id;
    // 题目标题会展示在列表和详情页，写入前由请求层完成非空校验。
    private String title;
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
