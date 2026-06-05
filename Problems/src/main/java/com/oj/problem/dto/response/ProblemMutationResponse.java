// 题目模块：该文件负责具体的数据结构、接口或业务逻辑
package com.oj.problem.dto.response;

import java.time.LocalDateTime;

// 对外暴露的方法或字段，通常承接模块间协作
public class ProblemMutationResponse {

    // 创建或更新题目后，前端回显用这几个字段就够了
    private Long id;
    private String title;
    // 内部实现细节，避免直接暴露给外部调用方
    private LocalDateTime createdAt;
    // 内部实现细节，避免直接暴露给外部调用方
    private LocalDateTime updatedAt;

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
    public String getTitle() {
        // 返回本阶段计算结果，供上层流程继续使用
        return title;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setTitle(String title) {
        this.title = title;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public LocalDateTime getCreatedAt() {
        // 返回本阶段计算结果，供上层流程继续使用
        return createdAt;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setCreatedAt(LocalDateTime createdAt) {
        this.createdAt = createdAt;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public LocalDateTime getUpdatedAt() {
        // 返回本阶段计算结果，供上层流程继续使用
        return updatedAt;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setUpdatedAt(LocalDateTime updatedAt) {
        this.updatedAt = updatedAt;
    }
}
