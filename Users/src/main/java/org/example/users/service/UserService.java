package org.example.users.service;

import org.example.users.entity.User;

public interface UserService {
    // 检验用户名、密码是否匹配，若匹配，返回用户
    User authenticate(String username, String password);
    // 检验用户名是否已被占用；若已被占用，返回 true
    boolean checkUsername(String username);
    // 将用户名、密码插入数据库
    void insertUser(String username, String password);
}