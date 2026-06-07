package com.oj.problem.service.impl;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.node.ArrayNode;
import com.fasterxml.jackson.databind.node.ObjectNode;
import com.oj.problem.config.JudgeProperties;
import com.oj.problem.dto.request.SubmitCodeRequest;
import com.oj.problem.dto.response.ProblemDetailResponse;
import com.oj.problem.dto.response.SubmitCodeResponse;
import com.oj.problem.dto.response.TestCaseResponse;
import com.oj.problem.exception.BusinessException;
import com.oj.problem.service.JudgeService;
import com.oj.problem.service.ProblemService;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.TimeUnit;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Service;
import org.springframework.util.StringUtils;

@Service
public class JudgeServiceImpl implements JudgeService {

    private static final Logger log = LoggerFactory.getLogger(JudgeServiceImpl.class);
    private static final Pattern RESULT_FILE_PATTERN =
            Pattern.compile("(?:Result|Error) json saved to:\\s*(.+)", Pattern.CASE_INSENSITIVE);
    private static final Pattern OJ_WEB_URL_PATTERN =
            Pattern.compile("OJ_WEB_URL\\s*=\\s*(https?://[^\\s\"'<>]+)", Pattern.CASE_INSENSITIVE);
    private static final Pattern URL_PATTERN =
            Pattern.compile("https?://[^\\s\"'<>]+", Pattern.CASE_INSENSITIVE);

    private final ProblemService problemService;
    private final JudgeProperties judgeProperties;
    private final ObjectMapper objectMapper;

    public JudgeServiceImpl(
            ProblemService problemService,
            JudgeProperties judgeProperties,
            ObjectMapper objectMapper) {
        this.problemService = problemService;
        this.judgeProperties = judgeProperties;
        this.objectMapper = objectMapper;
    }

    @Override
    public SubmitCodeResponse submit(Long problemId, SubmitCodeRequest request) {
        ProblemDetailResponse problem = problemService.getProblemDetail(problemId);
        if (problem.getTestCases() == null || problem.getTestCases().isEmpty()) {
            throw new BusinessException(400010, "题目没有测试用例", HttpStatus.BAD_REQUEST);
        }

        Path workDir = judgeProperties.resolveWorkDir();
        Path executable = workDir.resolve(judgeProperties.getExecutable());
        if (!Files.isRegularFile(executable)) {
            throw new BusinessException(
                    500010,
                    "评测引擎不存在：" + executable,
                    HttpStatus.INTERNAL_SERVER_ERROR);
        }

        String safeUsername = sanitizeUsername(request.getUsername());
        String prefix = safeUsername + "_" + System.currentTimeMillis();
        String srcFileName = sourceFileName(request.getLanguage());
        String casesFileName = prefix + "_cases.json";
        Path srcPath = workDir.resolve(prefix + "_" + srcFileName);
        Path casesPath = workDir.resolve(casesFileName);

        try {
            Files.createDirectories(workDir);
            Files.write(srcPath, request.getCode().getBytes(StandardCharsets.UTF_8));
            Files.write(casesPath, buildExpectJson(problem).getBytes(StandardCharsets.UTF_8));

            String srcArg = ".\\" + srcPath.getFileName();
            String casesArg = ".\\" + casesPath.getFileName();
            List<String> command = new ArrayList<>();
            command.add(executable.toString());
            command.add("--program_language=" + request.getLanguage());
            command.add("--src_file=" + srcArg);
            command.add("--expect_result=" + casesArg);
            command.add("--username=" + request.getUsername());

            log.info("Running judge: workDir={}, command={}", workDir, command);

            ProcessBuilder processBuilder = new ProcessBuilder(command);
            processBuilder.directory(workDir.toFile());
            processBuilder.redirectErrorStream(true);
            Process process = processBuilder.start();

            StringBuilder outputBuilder = new StringBuilder();
            Thread readerThread = new Thread(() -> {
                try (BufferedReader reader = new BufferedReader(
                        new InputStreamReader(process.getInputStream(), StandardCharsets.UTF_8))) {
                    String line;
                    while ((line = reader.readLine()) != null) {
                        outputBuilder.append(line).append(System.lineSeparator());
                    }
                } catch (IOException ex) {
                    log.warn("Failed to read judge output", ex);
                }
            });
            readerThread.start();

            boolean finished = process.waitFor(judgeProperties.getTimeoutSeconds(), TimeUnit.SECONDS);
            if (!finished) {
                process.destroyForcibly();
                readerThread.join(2000);
                throw new BusinessException(504010, "评测超时", HttpStatus.GATEWAY_TIMEOUT);
            }
            readerThread.join(5000);

            String output = outputBuilder.toString();
            int exitCode = process.exitValue();
            String resultFile = extractResultFile(output);
            String resultUrl = extractResultUrl(output);
            if (!StringUtils.hasText(resultUrl) && StringUtils.hasText(resultFile)) {
                resultUrl = buildResultUrl(resultFile);
            }
            if (exitCode != 0 && !StringUtils.hasText(resultFile) && !StringUtils.hasText(resultUrl)) {
                throw new BusinessException(
                        500011,
                        "评测引擎执行失败：" + trimOutput(output),
                        HttpStatus.INTERNAL_SERVER_ERROR);
            }

            SubmitCodeResponse response = new SubmitCodeResponse();
            response.setUsername(request.getUsername());
            response.setProblemId(problemId);
            response.setLanguage(request.getLanguage());
            response.setMessage(StringUtils.hasText(resultUrl) ? "提交成功，请点击链接查看评测结果" : "提交成功");
            response.setResultFile(resultFile);
            response.setResultUrl(resultUrl);
            return response;
        } catch (BusinessException ex) {
            throw ex;
        } catch (IOException ex) {
            log.error("Failed to prepare judge files", ex);
            throw new BusinessException(500012, "准备评测文件失败", HttpStatus.INTERNAL_SERVER_ERROR);
        } catch (InterruptedException ex) {
            Thread.currentThread().interrupt();
            throw new BusinessException(500013, "评测过程被中断", HttpStatus.INTERNAL_SERVER_ERROR);
        } finally {
            deleteQuietly(srcPath);
            deleteQuietly(casesPath);
        }
    }

    private String buildExpectJson(ProblemDetailResponse problem) throws IOException {
        ObjectNode root = objectMapper.createObjectNode();
        root.put("time_limit_ms", problem.getTimeLimit() == null ? 1000 : problem.getTimeLimit());
        root.put("memory_limit_mb", problem.getMemoryLimit() == null ? 128 : problem.getMemoryLimit());
        ArrayNode testCases = root.putArray("test_cases");
        int index = 1;
        for (TestCaseResponse testCase : problem.getTestCases()) {
            ObjectNode one = testCases.addObject();
            one.put("id", testCase.getId() == null ? index : testCase.getId());
            one.put("input", testCase.getInput() == null ? "" : testCase.getInput());
            one.put("expected_output", testCase.getOutput() == null ? "" : testCase.getOutput());
            one.put("score", testCase.getScore() == null ? 0 : testCase.getScore());
            index++;
        }
        return objectMapper.writerWithDefaultPrettyPrinter().writeValueAsString(root);
    }

    private String sourceFileName(String language) {
        switch (language) {
            case "java":
                return "Main.java";
            case "cpp":
                return "main.cpp";
            case "c":
                return "main.c";
            case "python":
                return "main.py";
            default:
                throw new BusinessException(400011, "不支持的编程语言", HttpStatus.BAD_REQUEST);
        }
    }

    private String sanitizeUsername(String username) {
        return username.trim().replaceAll("[^a-zA-Z0-9._-]", "_");
    }

    private String extractResultFile(String output) {
        if (!StringUtils.hasText(output)) {
            return null;
        }
        Matcher matcher = RESULT_FILE_PATTERN.matcher(output);
        if (matcher.find()) {
            return matcher.group(1).trim();
        }
        return null;
    }

    private String extractResultUrl(String output) {
        if (!StringUtils.hasText(output)) {
            return null;
        }
        Matcher webUrlMatcher = OJ_WEB_URL_PATTERN.matcher(output);
        if (webUrlMatcher.find()) {
            return webUrlMatcher.group(1).trim();
        }
        Matcher urlMatcher = URL_PATTERN.matcher(output);
        if (urlMatcher.find()) {
            return urlMatcher.group().trim();
        }
        return null;
    }

    private String buildResultUrl(String resultFile) {
        String normalized = resultFile == null ? "" : resultFile.trim();
        if (!StringUtils.hasText(normalized)) {
            return null;
        }
        String unixStyle = normalized.replace('\\', '/');
        int reportsIndex = unixStyle.toLowerCase().indexOf("/reports/");
        if (reportsIndex >= 0) {
            String relative = unixStyle.substring(reportsIndex + 1);
            return "http://localhost:8080/" + relative;
        }
        Path filePath = Paths.get(normalized).normalize();
        for (int i = 0; i < filePath.getNameCount(); i++) {
            if ("reports".equalsIgnoreCase(filePath.getName(i).toString())) {
                Path relative = filePath.subpath(i, filePath.getNameCount());
                return "http://localhost:8080/" + relative.toString().replace('\\', '/');
            }
        }
        return null;
    }

    private String trimOutput(String output) {
        if (!StringUtils.hasText(output)) {
            return "无输出";
        }
        String trimmed = output.trim();
        return trimmed.length() > 300 ? trimmed.substring(0, 300) + "..." : trimmed;
    }

    private void deleteQuietly(Path path) {
        if (path == null) {
            return;
        }
        try {
            Files.deleteIfExists(path);
        } catch (IOException ex) {
            log.warn("Failed to delete temp file: {}", path, ex);
        }
    }
}
