package org.example.users.service.impl;

import org.example.users.entity.User;
import org.example.users.repository.UserRepository;
import org.example.users.service.UserService;
import org.springframework.stereotype.Service;

@Service
public class UserServiceImpl implements UserService {
    private final UserRepository userRepository;

    public UserServiceImpl(UserRepository userRepository) {
        this.userRepository = userRepository;
    }

    @Override
    public User authenticate(String username,  String password) {
        if (checkUsername(username)) {
            User user = userRepository.findByUsername(username);
            if (user.getPassword().equals(password)) {
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
        userRepository.save(new User(username, password));
    }
}