package org.example.users.entity;

import jakarta.persistence.*;
import lombok.Getter;
import lombok.Setter;

@Setter
@Getter
@Entity
@Table(name = "Profiles")
public class Profile {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    // 关联 Users 中的 username
    @Column(unique = true, nullable = false)
    private String username;

    private String nickname;
    private String signature;
    // 头像的存储路径
    private String avatar;

    public Profile() {}

    public Profile(String username) {
        this.username = username;
    }
}