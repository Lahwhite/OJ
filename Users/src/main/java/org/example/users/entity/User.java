package org.example.users.entity;

import jakarta.persistence.*;
import lombok.Getter;
import lombok.Setter;

@Setter
@Getter
@Entity
@Table(name = "Users")
public class User {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;
    private String username;
    private String password;

    // User 的构造函数
    public User() {}

    public User(String username, String password) {
        this.username = username;
        this.password = password;
    }
}