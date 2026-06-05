// 题目模块：该文件负责具体的数据结构、接口或业务逻辑
package com.oj.problem;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;

@SpringBootApplication
// 对外暴露的方法或字段，通常承接模块间协作
public class ProblemManagementApplication {

    // Problems 模块的 Spring Boot 启动入口
    public static void main(String[] args) {
        SpringApplication.run(ProblemManagementApplication.class, args);
    }
}
