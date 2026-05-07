package org.example.users.service.impl;

import org.example.users.entity.Profile;
import org.example.users.repository.ProfileRepository;
import org.example.users.service.ProfileService;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

@Service
public class ProfileServiceImpl implements ProfileService {
    private final ProfileRepository profileRepository;

    public ProfileServiceImpl(ProfileRepository profileRepository) {
        this.profileRepository = profileRepository;
    }

    @Override
    public Profile getProfileByUsername(String username) {
        return profileRepository.findByUsername(username);
    }

    @Override
    @Transactional
    public void updateProfileByUsername(String username, String nickname, String signature, String avatar) {
        Profile profile = profileRepository.findByUsername(username);
        if (profile == null) {
            profile = new Profile(username);
        }
        if (nickname != null) {
            profile.setNickname(nickname);
        }
        if (signature != null) {
            profile.setSignature(signature);
        }
        if (avatar != null) {
            profile.setAvatar(avatar);
        }
        // 不存在就插入；存在就更新
        profileRepository.save(profile);
    }
}