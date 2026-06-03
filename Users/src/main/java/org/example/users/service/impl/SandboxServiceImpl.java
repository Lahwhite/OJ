package org.example.users.service.impl;

import lombok.extern.slf4j.Slf4j;
import org.example.users.constants.SandboxConstant;
import org.example.users.dto.CodeRunRequest;
import org.example.users.dto.CodeRunResponse;
import org.example.users.enums.CodeLanguage;
import org.example.users.service.SandboxService;
import org.springframework.stereotype.Service;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

@Slf4j
@Service
public class SandboxServiceImpl implements SandboxService {
    private final Semaphore semaphore = new Semaphore(SandboxConstant.MAX_CONCURRENT_RUNS);

    @Override
    public CodeRunResponse runCode(CodeRunRequest request){
        CodeRunResponse response = new CodeRunResponse();
        try {
            if (!semaphore.tryAcquire(2, TimeUnit.SECONDS)) {
                response.setSuccess(false);
                response.setErrorMessage("当前系统繁忙，请稍后重试");
                return response;
            }
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            response.setSuccess(false);
            response.setErrorMessage("系统繁忙，请求被中断");
            return response;
        }

        try {
            File codeFile = null;
            Process process = null;

            // 源码长度校验
            if (request.getCode() != null && request.getCode().length() > SandboxConstant.MAX_CODE_LENGTH) {
                response.setSuccess(false);
                response.setErrorMessage("代码长度超出限制（最大 " + SandboxConstant.MAX_CODE_LENGTH / 1024 + " KB）");
                return response;
            }

            try {
                // 校验语言并获取配置
                CodeLanguage language = CodeLanguage.valueOf(request.getLanguage().toUpperCase());

                // 生成临时代码文件（使用 UUID 避免冲突）
                String uuid = UUID.randomUUID().toString();
                Path tmpDir = Path.of(SandboxConstant.TEMP_DIR);
                Files.createDirectories(tmpDir);
                Path codeFilePath = tmpDir.resolve(uuid + "_" + language.getFileName());
                Files.writeString(codeFilePath, request.getCode(), StandardCharsets.UTF_8);
                codeFile = codeFilePath.toFile();
                codeFile.deleteOnExit();

                // 构建 Docker 命令（使用数组避免 Shell 注入）
                List<String> cmd = getStrings(codeFile, language);

                log.debug("执行 Docker 命令：{}", String.join(" ", cmd));

                // 启动一个外部进程并将进程的标准错误流（stderr）合并到标准输出流（stdout）
                ProcessBuilder pb = new ProcessBuilder(cmd);
                pb.redirectErrorStream(true);
                process = pb.start();

                // 启动读取线程
                ByteArrayOutputStream out = new ByteArrayOutputStream();
                Process finalProcess = process;
                Thread readerThread = new Thread(() -> {
                    try (InputStream in = finalProcess.getInputStream()) {
                        byte[] buf = new byte[8192];
                        int total = 0, len;
                        while ((len = in.read(buf)) != -1) {
                            total += len;
                            if (total > SandboxConstant.MAX_OUTPUT_BYTES) {
                                out.write(buf, 0, len - (total - SandboxConstant.MAX_OUTPUT_BYTES));
                                out.write("\n[输出已截断]".getBytes(StandardCharsets.UTF_8));
                                // 继续读完剩余数据并丢弃，防止阻塞
                                while (in.read(buf) != -1) {}
                                break;
                            }
                            out.write(buf, 0, len);
                        }
                    } catch (IOException ignored) { }
                });
                readerThread.start();

                // 根据语言设置 timeout
                int timeout = (language == CodeLanguage.JAVA)
                        ? SandboxConstant.TIMEOUT_JAVA
                        : SandboxConstant.TIMEOUT_DEFAULT;
                boolean finished = process.waitFor(timeout, TimeUnit.SECONDS);
                if (!finished) {
                    process.destroyForcibly();
                    readerThread.join(2000);
                    String partialOutput = out.toString(StandardCharsets.UTF_8);
                    response.setOutput(partialOutput.isEmpty() ? null : partialOutput);

                    response.setSuccess(false);
                    boolean terminated = process.waitFor(2, TimeUnit.SECONDS);
                    if (!terminated) {
                        // 强制终止后进程都没消失，不再尝试读取输出，直接返回错误
                        response.setErrorMessage("运行超时且无法强制终止");
                    }
                    else {
                        response.setErrorMessage("运行超时（限制" + timeout + "秒）");
                    }
                }
                else {
                    readerThread.join(2000);
                    String output = out.toString(StandardCharsets.UTF_8);

                    int exitCode = process.exitValue();
                    response.setSuccess(exitCode == 0);
                    response.setOutput(output);
                    if (exitCode != 0) {
                        response.setErrorMessage("进程退出码：" + exitCode);
                    }
                }
            } catch (IllegalArgumentException  e) {
                response.setSuccess(false);
                response.setErrorMessage("不支持的编程语言：" + request.getLanguage());
                log.warn("不支持的编程语言：{}", request.getLanguage(), e);
            } catch (Exception e) {
                response.setSuccess(false);
                response.setErrorMessage("系统内部错误，请联系管理员");
                log.error("沙箱运行异常", e);
            } finally {
                if (process != null) {
                    process.destroyForcibly();
                    try {
                        process.waitFor(2, TimeUnit.SECONDS);
                    } catch (InterruptedException ignored) {
                        Thread.currentThread().interrupt();
                    }
                }
                if (codeFile != null && codeFile.exists()) {
                    try {
                        Files.deleteIfExists(codeFile.toPath());
                    } catch (IOException e) {
                        log.warn("临时文件删除失败：{}", codeFile.getAbsolutePath(), e);
                    }
                }
            }
        } finally {
            // 释放信号量
            semaphore.release();
        }

        return response;
    }

    private static List<String> getStrings(File codeFile, CodeLanguage language) {
        List<String> cmd = new ArrayList<>();
        cmd.add("docker");
        cmd.add("run");
        cmd.add("--rm");

        cmd.add("--memory=" + SandboxConstant.MEMORY_LIMIT);
        cmd.add("--cpus=" + SandboxConstant.CPU_CORES_LIMIT);
        cmd.add("--network=" + SandboxConstant.NETWORK_MODE);

        cmd.add("--read-only");
        cmd.add("-v");
        // /app 目录只读
        cmd.add(codeFile.getAbsolutePath() + ":/app/" + language.getFileName() + ":ro");

        cmd.add("--security-opt=no-new-privileges");
        cmd.add("--user=1000:1000");

        // 根据语言设置 pids-limit
        int pidsLimit = (language == CodeLanguage.JAVA)
                ? SandboxConstant.PIDS_LIMIT_JAVA
                : SandboxConstant.PIDS_LIMIT_DEFAULT;
        cmd.add("--pids-limit=" + pidsLimit);

        // /tmp 目录可写不可执行，/run 目录可写可执行
        cmd.add("--tmpfs=/tmp:rw,noexec,nosuid,size=" + SandboxConstant.DISK_LIMIT);
        cmd.add("--tmpfs=/exec:rw,exec,nosuid,uid=1000,gid=1000,size=" + SandboxConstant.DISK_LIMIT);

        cmd.add("-w");
        cmd.add("/tmp");
        cmd.add(language.getImage());
        cmd.add("sh");
        cmd.add("-c");
        cmd.add(language.getRunCmd());
        return cmd;
    }
}
