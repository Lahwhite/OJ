package com.oj.problem.security;

public class CurrentUser {

    private final Long userId;
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
