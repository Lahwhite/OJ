package com.oj.problem.service.impl;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertTrue;

import com.oj.problem.dto.request.ProblemQueryRequest;
import com.oj.problem.dto.request.ProblemUpsertRequest;
import com.oj.problem.dto.request.TestCaseRequest;
import com.oj.problem.dto.response.ProblemDetailResponse;
import com.oj.problem.dto.response.ProblemPageResponse;
import com.oj.problem.entity.Difficulty;
import com.oj.problem.repository.ProblemRepository;
import com.oj.problem.security.CurrentUser;
import com.oj.problem.service.ProblemService;
import java.util.Arrays;
import java.util.Collections;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.test.context.ActiveProfiles;
import org.springframework.transaction.annotation.Transactional;

@SpringBootTest
@ActiveProfiles("test")
@Transactional
class ProblemServiceIntegrationTest {

    @Autowired
    private ProblemService problemService;

    @Autowired
    private ProblemRepository problemRepository;

    @Test
    void createQueryUpdateStatsAndDeleteShouldWorkWithDatabase() {
        CurrentUser admin = new CurrentUser(1L, "admin");
        ProblemUpsertRequest request = buildRequest("数据库流程题", Arrays.asList("数组", "模拟"));

        Long problemId = problemService.createProblem(request, admin).getId();

        ProblemQueryRequest query = new ProblemQueryRequest();
        query.setKeyword("数据库");
        query.setTag("数组");
        ProblemPageResponse page = problemService.listProblems(query);
        assertEquals(1L, page.getTotal());
        assertEquals("数据库流程题", page.getProblems().get(0).getTitle());

        ProblemDetailResponse detail = problemService.getProblemDetail(problemId);
        assertEquals(1, detail.getTestCases().size());
        assertTrue(detail.getTestCases().get(0).getIsSample());

        problemService.recordSubmissionResult(problemId, true);
        ProblemDetailResponse afterAccepted = problemService.getProblemDetail(problemId);
        assertEquals(1, afterAccepted.getSubmissionCount());
        assertEquals(1, afterAccepted.getAcceptedCount());
        assertEquals(1D, afterAccepted.getPassRate());

        ProblemUpsertRequest updated = buildRequest("数据库流程题更新", Collections.singletonList("模拟"));
        problemService.updateProblem(problemId, updated, admin);
        ProblemDetailResponse afterUpdate = problemService.getProblemDetail(problemId);
        assertEquals("数据库流程题更新", afterUpdate.getTitle());
        assertEquals(Collections.singletonList("模拟"), afterUpdate.getTags());

        problemService.deleteProblem(problemId, admin);
        assertFalse(problemRepository.findById(problemId).isPresent());
    }

    private ProblemUpsertRequest buildRequest(String title, java.util.List<String> tags) {
        ProblemUpsertRequest request = new ProblemUpsertRequest();
        request.setTitle(title);
        request.setDescription("给定若干整数，输出它们的和。");
        request.setDifficulty(Difficulty.easy);
        request.setTimeLimit(1000);
        request.setMemoryLimit(128);
        request.setInputDescription("第一行整数个数，第二行若干整数。");
        request.setOutputDescription("输出所有整数之和。");
        request.setSampleInput("3\n1 2 3");
        request.setSampleOutput("6");
        request.setTags(tags);
        request.setIsPublic(true);
        request.setTestCases(Arrays.asList(
                buildTestCase("3\n1 2 3", "6", true, 20),
                buildTestCase("4\n1 1 1 1", "4", false, 80)
        ));
        return request;
    }

    private TestCaseRequest buildTestCase(String input, String output, boolean sample, int score) {
        TestCaseRequest request = new TestCaseRequest();
        request.setInput(input);
        request.setOutput(output);
        request.setIsSample(sample);
        request.setScore(score);
        return request;
    }
}
