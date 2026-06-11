package com.oj.problem.security;

public class CurrentUser {

    // 用户模块传入的用户 ID，用于区分不同用户的做题状态。
    private final Long userId;
    // 用户角色，当前模块主要用来区分管理员和普通用户。
    private final String role;

    // 从 JWT token 里解析出来的当前登录用户信息
    public CurrentUser(Long userId, String role) {
        this.userId = userId;
        this.role = role;
    }

    public Long getUserId() {
        return userId;
    }

    public String getRole() {
        return role;
    }

    public boolean isAdmin() {
        return "admin".equalsIgnoreCase(role);
    }
}
