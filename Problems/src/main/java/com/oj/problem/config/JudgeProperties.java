package com.oj.problem.config;

import java.nio.file.Path;
import java.nio.file.Paths;
import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.stereotype.Component;

@Component
@ConfigurationProperties(prefix = "judge")
public class JudgeProperties {

    // 评测工作目录，源码、用例和结果文件都相对该目录处理。
    private String workDir = "../judge";
    // 评测引擎可执行文件名，启动前会检查文件是否存在。
    private String executable = "judge_engine.exe";
    // 评测进程最大等待时间，超时后强制结束进程。
    private int timeoutSeconds = 120;

    public Path resolveWorkDir() {
        return Paths.get(workDir).toAbsolutePath().normalize();
    }

    public String getWorkDir() {
        return workDir;
    }

    public void setWorkDir(String workDir) {
        this.workDir = workDir;
    }

    public String getExecutable() {
        return executable;
    }

    public void setExecutable(String executable) {
        this.executable = executable;
    }

    public int getTimeoutSeconds() {
        return timeoutSeconds;
    }

    public void setTimeoutSeconds(int timeoutSeconds) {
        this.timeoutSeconds = timeoutSeconds;
    }
}
