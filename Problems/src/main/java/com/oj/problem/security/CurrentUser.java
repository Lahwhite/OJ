// 题目模块：该文件负责具体的数据结构、接口或业务逻辑
package com.oj.problem.security;

// 对外暴露的方法或字段，通常承接模块间协作
public class CurrentUser {

    // 内部实现细节，避免直接暴露给外部调用方
    private final Long userId;
    // 内部实现细节，避免直接暴露给外部调用方
    private final String role;

    // 从 JWT token 里解析出来的当前登录用户信息
    public CurrentUser(Long userId, String role) {
        this.userId = userId;
        this.role = role;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public Long getUserId() {
        // 返回本阶段计算结果，供上层流程继续使用
        return userId;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public String getRole() {
        // 返回本阶段计算结果，供上层流程继续使用
        return role;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public boolean isAdmin() {
        // 返回本阶段计算结果，供上层流程继续使用
        return "admin".equalsIgnoreCase(role);
    }
}
