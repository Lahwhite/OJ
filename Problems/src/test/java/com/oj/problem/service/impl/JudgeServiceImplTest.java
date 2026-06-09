package com.oj.problem.service.impl;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.oj.problem.config.JudgeProperties;
import com.oj.problem.dto.request.SubmissionResultRequest;
import com.oj.problem.dto.request.SubmitCodeRequest;
import com.oj.problem.dto.response.ProblemDetailResponse;
import com.oj.problem.dto.response.SubmitCodeResponse;
import com.oj.problem.dto.response.TestCaseResponse;
import com.oj.problem.entity.ProblemUserEntity;
import com.oj.problem.repository.ProblemUserRepository;
import com.oj.problem.service.ProblemService;
import com.oj.problem.service.ProblemStatusService;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Collections;
import java.util.Optional;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;
import org.mockito.ArgumentCaptor;

class JudgeServiceImplTest {

    @TempDir
    private Path tempDir;

    @Test
    void submitShouldReadJudgeResultJsonAndUpdateProblemStatus() throws Exception {
        Path resultPath = tempDir.resolve("alice_judge_result_1.json");
        Files.write(resultPath, Collections.singletonList(
                "{\"verdict\":\"AC\",\"total_score\":100,\"max_score\":100}"),
                StandardCharsets.UTF_8);

        String executableName = createFakeJudgeExecutable(resultPath);

        JudgeProperties properties = new JudgeProperties();
        properties.setWorkDir(tempDir.toString());
        properties.setExecutable(executableName);
        properties.setTimeoutSeconds(5);

        ProblemService problemService = org.mockito.Mockito.mock(ProblemService.class);
        ProblemStatusService statusService = org.mockito.Mockito.mock(ProblemStatusService.class);
        ProblemUserRepository userRepository = org.mockito.Mockito.mock(ProblemUserRepository.class);

        ProblemDetailResponse problem = problemWithOneTestCase();
        when(problemService.getProblemDetail(7L)).thenReturn(problem);
        when(userRepository.findByUsername("alice")).thenReturn(Optional.empty());
        when(userRepository.save(any(ProblemUserEntity.class))).thenAnswer(invocation -> {
            ProblemUserEntity user = invocation.getArgument(0);
            user.setId(42L);
            return user;
        });

        JudgeServiceImpl judgeService = new JudgeServiceImpl(
                problemService,
                statusService,
                userRepository,
                properties,
                new ObjectMapper());

        SubmitCodeRequest request = new SubmitCodeRequest();
        request.setUsername("alice");
        request.setLanguage("python");
        request.setCode("print(1)");

        SubmitCodeResponse response = judgeService.submit(7L, request);

        assertEquals(42L, response.getUserId());
        assertEquals("AC", response.getVerdict());
        assertTrue(response.getAccepted());
        assertEquals(100, response.getScore());
        assertEquals(100, response.getMaxScore());

        ArgumentCaptor<SubmissionResultRequest> captor = ArgumentCaptor.forClass(SubmissionResultRequest.class);
        verify(statusService).upsertStatus(captor.capture());
        SubmissionResultRequest statusRequest = captor.getValue();
        assertEquals(42L, statusRequest.getUserId());
        assertEquals(7L, statusRequest.getProblemId());
        assertTrue(statusRequest.getAccepted());
        assertEquals(100, statusRequest.getScore());
        assertEquals(100, statusRequest.getMaxScore());
    }

    private String createFakeJudgeExecutable(Path resultPath) throws Exception {
        boolean windows = System.getProperty("os.name").toLowerCase().contains("win");
        if (windows) {
            Path script = tempDir.resolve("fake-judge.cmd");
            Files.write(script, Collections.singletonList(
                    "@echo Result json saved to: " + resultPath.toString()),
                    StandardCharsets.UTF_8);
            return script.getFileName().toString();
        }

        Path script = tempDir.resolve("fake-judge.sh");
        Files.write(script, Collections.singletonList(
                "#!/bin/sh\necho \"Result json saved to: " + resultPath.toString() + "\""),
                StandardCharsets.UTF_8);
        script.toFile().setExecutable(true);
        return script.getFileName().toString();
    }

    private ProblemDetailResponse problemWithOneTestCase() {
        TestCaseResponse testCase = new TestCaseResponse();
        testCase.setId(1L);
        testCase.setInput("1 2\n");
        testCase.setOutput("3\n");
        testCase.setScore(100);

        ProblemDetailResponse problem = new ProblemDetailResponse();
        problem.setId(7L);
        problem.setTimeLimit(1000);
        problem.setMemoryLimit(128);
        problem.setTestCases(Collections.singletonList(testCase));
        return problem;
    }
}
