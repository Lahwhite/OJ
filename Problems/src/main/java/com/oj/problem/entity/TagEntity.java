// 题目模块：该文件负责具体的数据结构、接口或业务逻辑
package com.oj.problem.entity;

import java.time.LocalDateTime;
import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.GeneratedValue;
import javax.persistence.GenerationType;
import javax.persistence.Id;
import javax.persistence.PrePersist;
import javax.persistence.Table;

@Entity
@Table(name = "tags")
// 对外暴露的方法或字段，通常承接模块间协作
public class TagEntity {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    // 内部实现细节，避免直接暴露给外部调用方
    private Long id;

    // 标签名全局唯一，前端筛选会直接用到
    @Column(nullable = false, unique = true, length = 50)
    private String name;

    @Column(length = 7, nullable = false)
    // 内部实现细节，避免直接暴露给外部调用方
    private String color = "#1890ff";

    @Column(name = "problem_count", nullable = false)
    // 内部实现细节，避免直接暴露给外部调用方
    private Integer problemCount = 0;

    @Column(name = "created_at", nullable = false)
    // 内部实现细节，避免直接暴露给外部调用方
    private LocalDateTime createdAt;

    @PrePersist
    // 对外暴露的方法或字段，通常承接模块间协作
    public void onCreate() {
        this.createdAt = LocalDateTime.now();
    }

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

    // 对外暴露的方法或字段，通常承接模块间协作
    public LocalDateTime getCreatedAt() {
        // 返回本阶段计算结果，供上层流程继续使用
        return createdAt;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setCreatedAt(LocalDateTime createdAt) {
        this.createdAt = createdAt;
    }
}
