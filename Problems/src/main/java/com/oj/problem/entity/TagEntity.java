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
public class TagEntity {

    // 数据库主键，所有对外响应都沿用这个标识定位资源。
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    // 标签名全局唯一，前端筛选会直接用到
    @Column(nullable = false, unique = true, length = 50)
    private String name;

    // 标签颜色预留字段，目前主要用于前端扩展展示。
    @Column(length = 7, nullable = false)
    private String color = "#1890ff";

    // 使用该标签的题目数量，同步标签时增减。
    @Column(name = "problem_count", nullable = false)
    private Integer problemCount = 0;

    // 创建时间由数据库自动维护，用于列表排序和审计展示。
    @Column(name = "created_at", nullable = false)
    private LocalDateTime createdAt;

    @PrePersist
    public void onCreate() {
        this.createdAt = LocalDateTime.now();
    }

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

    public LocalDateTime getCreatedAt() {
        return createdAt;
    }

    public void setCreatedAt(LocalDateTime createdAt) {
        this.createdAt = createdAt;
    }
}
