package org.example.users.service;

import org.example.users.entity.Profile;

public interface ProfileService {
    // 获取当前用户的个人资料
    Profile getProfileByUsername(String username);
    // 更新当前用户的个人资料
    void updateProfileByUsername(String username, String nickname, String signature, String avatar);
}