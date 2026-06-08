package org.example.users.service.impl;

import org.example.users.entity.ProblemUser;
import org.example.users.entity.User;
import org.example.users.repository.ProblemUserRepository;
import org.example.users.repository.UserRepository;
import org.example.users.service.UserService;
import org.springframework.security.crypto.bcrypt.BCryptPasswordEncoder;
import org.springframework.stereotype.Service;

@Service
public class UserServiceImpl implements UserService {
    private final UserRepository userRepository;
    private final ProblemUserRepository problemUserRepository;
    private static final BCryptPasswordEncoder ENCODER = new BCryptPasswordEncoder();

    public UserServiceImpl(UserRepository userRepository, ProblemUserRepository problemUserRepository) {
        this.userRepository = userRepository;
        this.problemUserRepository = problemUserRepository;
    }

    @Override
    public User authenticate(String username,  String password) {
        if (checkUsername(username)) {
            User user = userRepository.findByUsername(username);
            // 登录时：校验用户输入的密码和数据库里的散列值
            if (ENCODER.matches(password, user.getPassword())) {
                return user;
            }
        }
        return null;
    }

    @Override
    public boolean checkUsername(String username) {
        if (username != null) {
            return userRepository.existsByUsername(username);
        }
        return false;
    }

    @Override
    public void insertUser(String username, String password) {
        // 注册时：对密码加密，存到数据库
        String hashed = ENCODER.encode(password);
        userRepository.save(new User(username, hashed));
    }

    //
    @Override
    public Long getBindUserId(String username) {
        // 直接通过 username 查询 problem_users 表，该字段有唯一约束
        ProblemUser problemUser = problemUserRepository.findByUsername(username);
        // 存在则返回其 ID，否则返回 null
        return problemUser != null ? problemUser.getId() : null;
    }
}