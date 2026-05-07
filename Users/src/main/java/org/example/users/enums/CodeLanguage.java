package org.example.users.enums;

import lombok.Getter;

@Getter
public enum CodeLanguage {
    PYTHON("python:3.11-slim", "main.py", "python /app/main.py"),
    C("gcc:12.2", "main.c", "gcc /app/main.c -o /exec/prog && /exec/prog"),
    CPP("gcc:12.2", "main.cpp", "g++ /app/main.cpp -o /exec/prog && /exec/prog"),
    JAVA("openjdk:17-slim", "Main.java", "java /app/Main.java");

    private final String image;
    private final String fileName;
    private final String runCmd;

    CodeLanguage(String image, String fileName, String runCmd) {
        this.image = image;
        this.fileName = fileName;
        this.runCmd = runCmd;
    }
}
