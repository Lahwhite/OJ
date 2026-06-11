package com.oj.problem.service.impl;

import com.oj.problem.dto.request.ProblemQueryRequest;
import com.oj.problem.dto.request.ProblemUpsertRequest;
import com.oj.problem.dto.request.TestCaseRequest;
import com.oj.problem.dto.response.ProblemDetailResponse;
import com.oj.problem.dto.response.ProblemListItemResponse;
import com.oj.problem.dto.response.ProblemMutationResponse;
import com.oj.problem.dto.response.ProblemPageResponse;
import com.oj.problem.dto.response.TestCaseResponse;
import com.oj.problem.entity.ProblemEntity;
import com.oj.problem.entity.TagEntity;
import com.oj.problem.entity.TestCaseEntity;
import com.oj.problem.exception.BusinessException;
import com.oj.problem.repository.ProblemRepository;
import com.oj.problem.repository.TagRepository;
import com.oj.problem.security.CurrentUser;
import com.oj.problem.service.ProblemService;
import java.util.ArrayList;
import java.util.Collections;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Set;
import java.util.function.Function;
import java.util.stream.Collectors;
import javax.persistence.criteria.Join;
import javax.persistence.criteria.JoinType;
import javax.persistence.criteria.Predicate;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.PageRequest;
import org.springframework.data.domain.Pageable;
import org.springframework.data.domain.Sort;
import org.springframework.data.jpa.domain.Specification;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;
import org.springframework.util.StringUtils;

@Service
public class ProblemServiceImpl implements ProblemService {

    private final ProblemRepository problemRepository;
    private final TagRepository tagRepository;

    public ProblemServiceImpl(ProblemRepository problemRepository, TagRepository tagRepository) {
        this.problemRepository = problemRepository;
        this.tagRepository = tagRepository;
    }
    // 列表查询只返回公开题目，并把筛选条件转换成 JPA Specification。

    @Override
    @Transactional(readOnly = true)
    public ProblemPageResponse listProblems(ProblemQueryRequest queryRequest) {
        // 按创建时间倒序分页，page 从 1 开始但 PageRequest 从 0 开始所以要减 1
        Pageable pageable = PageRequest.of(
                queryRequest.getPage() - 1,
                queryRequest.getSize(),
                Sort.by(Sort.Direction.DESC, "createdAt"));

        Page<ProblemEntity> page = problemRepository.findAll(buildSpecification(queryRequest), pageable);
        ProblemPageResponse response = new ProblemPageResponse();
        response.setTotal(page.getTotalElements());
        response.setPage(queryRequest.getPage());
        response.setSize(queryRequest.getSize());
        response.setProblems(page.getContent().stream().map(this::toListItem).collect(Collectors.toList()));
        return response;
    }
    // 普通用户只能读取公开题目详情，测试用例和标签会一起加载。

    @Override
    @Transactional(readOnly = true)
    public ProblemDetailResponse getProblemDetail(Long id) {
        ProblemEntity problem = problemRepository.findWithTestCasesAndTagsById(id)
                .filter(ProblemEntity::getIsPublic)
                .orElseThrow(() -> new BusinessException(404002, "题目不存在", HttpStatus.NOT_FOUND));
        return toDetail(problem);
    }
    // 评测前读取题目用例，仍然通过公开题目校验避免越权访问。

    @Override
    @Transactional(readOnly = true)
    public List<TestCaseResponse> getProblemTestCases(Long id) {
        ProblemEntity problem = problemRepository.findWithTestCasesAndTagsById(id)
                .filter(ProblemEntity::getIsPublic)
                .orElseThrow(() -> new BusinessException(404002, "题目不存在", HttpStatus.NOT_FOUND));
        return problem.getTestCases().stream().map(this::toTestCase).collect(Collectors.toList());
    }
    // 新建题目时同时同步测试用例和标签，保证聚合根一次性保存。

    @Override
    @Transactional
    public ProblemMutationResponse createProblem(ProblemUpsertRequest request, CurrentUser currentUser) {
        ProblemEntity entity = new ProblemEntity();
        applyRequest(entity, request, currentUser);
        ProblemEntity saved = problemRepository.save(entity);
        return toMutation(saved);
    }
    // 更新题目复用同一套字段同步逻辑，避免创建和编辑行为分叉。

    @Override
    @Transactional
    public ProblemMutationResponse updateProblem(Long id, ProblemUpsertRequest request, CurrentUser currentUser) {
        ProblemEntity entity = problemRepository.findWithTestCasesAndTagsById(id)
                .orElseThrow(() -> new BusinessException(404002, "题目不存在", HttpStatus.NOT_FOUND));
        applyRequest(entity, request, currentUser);
        ProblemEntity saved = problemRepository.save(entity);
        return toMutation(saved);
    }
    // 评测结果回写题目统计，提交数总是递增，AC 时再递增通过数。

    @Override
    @Transactional
    public void recordSubmissionResult(Long id, boolean accepted) {
        ProblemEntity entity = problemRepository.findById(id)
                .orElseThrow(() -> new BusinessException(404002, "题目不存在", HttpStatus.NOT_FOUND));
        int submissionCount = entity.getSubmissionCount() == null ? 0 : entity.getSubmissionCount();
        int acceptedCount = entity.getAcceptedCount() == null ? 0 : entity.getAcceptedCount();
        entity.setSubmissionCount(submissionCount + 1);
        if (accepted) {
            entity.setAcceptedCount(acceptedCount + 1);
        }
        problemRepository.save(entity);
    }
    // 删除题目前先扣减标签引用数，再交给 JPA 级联清理测试用例。

    @Override
    @Transactional
    public void deleteProblem(Long id, CurrentUser currentUser) {
        ProblemEntity entity = problemRepository.findWithTestCasesAndTagsById(id)
                .orElseThrow(() -> new BusinessException(404002, "题目不存在", HttpStatus.NOT_FOUND));
        decreaseTagCount(entity.getTags());
        problemRepository.delete(entity);
    }
    // 动态拼装筛选条件，标签过滤需要 distinct 防止多标签连接产生重复题目。

    private Specification<ProblemEntity> buildSpecification(ProblemQueryRequest queryRequest) {
        return (root, query, cb) -> {
            List<Predicate> predicates = new ArrayList<>();
            predicates.add(cb.isTrue(root.get("isPublic")));

            if (queryRequest.getDifficulty() != null) {
                predicates.add(cb.equal(root.get("difficulty"), queryRequest.getDifficulty()));
            }

            if (StringUtils.hasText(queryRequest.getKeyword())) {
                String keyword = "%" + queryRequest.getKeyword().trim().toLowerCase(Locale.ROOT) + "%";
                predicates.add(cb.or(
                        cb.like(cb.lower(root.get("title")), keyword),
                        cb.like(cb.lower(root.get("description")), keyword)
                ));
            }

            if (StringUtils.hasText(queryRequest.getTag())) {
                Join<Object, Object> tagJoin = root.join("tags", JoinType.LEFT);
                predicates.add(cb.equal(cb.lower(tagJoin.get("name")), queryRequest.getTag().trim().toLowerCase(Locale.ROOT)));
                query.distinct(true);
            }

            return cb.and(predicates.toArray(new Predicate[0]));
        };
    }
    // 把请求 DTO 归一化写入实体，关联集合交给专门方法同步。

    private void applyRequest(ProblemEntity entity, ProblemUpsertRequest request, CurrentUser currentUser) {
        entity.setTitle(request.getTitle().trim());
        entity.setDescription(request.getDescription().trim());
        entity.setDifficulty(request.getDifficulty());
        entity.setTimeLimit(request.getTimeLimit());
        entity.setMemoryLimit(request.getMemoryLimit());
        entity.setInputDescription(request.getInputDescription().trim());
        entity.setOutputDescription(request.getOutputDescription().trim());
        entity.setSampleInput(request.getSampleInput());
        entity.setSampleOutput(request.getSampleOutput());
        entity.setIsPublic(Boolean.TRUE.equals(request.getIsPublic()));
        if (entity.getCreatedBy() == null) {
            entity.setCreatedBy(currentUser.getUserId());
        }

        syncTestCases(entity, request.getTestCases());
        syncTags(entity, request.getTags());
    }
    // 重建测试用例集合，让 orphanRemoval 接管旧用例删除。

    private void syncTestCases(ProblemEntity entity, List<TestCaseRequest> testCases) {
        entity.clearTestCases();
        for (TestCaseRequest request : testCases) {
            TestCaseEntity testCase = new TestCaseEntity();
            testCase.setInput(request.getInput());
            testCase.setOutput(request.getOutput());
            testCase.setIsSample(request.getIsSample());
            testCase.setScore(request.getScore());
            entity.addTestCase(testCase);
        }
    }
    // 按标签名称去重并复用已有标签，同时维护 problemCount。

    private void syncTags(ProblemEntity entity, List<String> requestedTags) {
        List<String> normalizedNames = requestedTags == null
                ? Collections.emptyList()
                : requestedTags.stream()
                        .filter(StringUtils::hasText)
                        .map(String::trim)
                        .distinct()
                        .collect(Collectors.toList());

        Set<TagEntity> oldTags = new LinkedHashSet<>(entity.getTags());
        List<TagEntity> existingTags = normalizedNames.isEmpty()
                ? Collections.emptyList()
                : tagRepository.findByNameIn(normalizedNames);
        Map<String, TagEntity> existingMap = existingTags.stream()
                .collect(Collectors.toMap(TagEntity::getName, Function.identity()));

        Set<TagEntity> newTags = new LinkedHashSet<>();
        Set<String> newTagNames = new LinkedHashSet<>();
        for (String name : normalizedNames) {
            TagEntity tag = existingMap.get(name);
            if (tag == null) {
                tag = new TagEntity();
                tag.setName(name);
                tag.setProblemCount(0);
            }
            newTags.add(tag);
            newTagNames.add(name);
        }

        Set<String> oldTagNames = oldTags.stream().map(TagEntity::getName).collect(Collectors.toSet());
        Set<TagEntity> tagsToAdd = newTags.stream()
                .filter(tag -> !oldTagNames.contains(tag.getName()))
                .collect(Collectors.toCollection(LinkedHashSet::new));
        Set<TagEntity> tagsToRemove = oldTags.stream()
                .filter(tag -> !newTagNames.contains(tag.getName()))
                .collect(Collectors.toCollection(LinkedHashSet::new));

        for (TagEntity tag : tagsToAdd) {
            tag.setProblemCount(tag.getProblemCount() + 1);
        }

        decreaseTagCount(tagsToRemove);

        entity.getTags().clear();
        entity.getTags().addAll(newTags);
    }
    // 标签引用数做下限保护，避免重复删除导致负数。

    private void decreaseTagCount(Set<TagEntity> tags) {
        for (TagEntity tag : tags) {
            int current = tag.getProblemCount() == null ? 0 : tag.getProblemCount();
            tag.setProblemCount(Math.max(0, current - 1));
        }
    }
    // 列表响应只保留摘要字段，避免把大题面和测试用例带到列表页。

    private ProblemListItemResponse toListItem(ProblemEntity entity) {
        ProblemListItemResponse response = new ProblemListItemResponse();
        response.setId(entity.getId());
        response.setTitle(entity.getTitle());
        response.setDifficulty(entity.getDifficulty().name());
        response.setTags(entity.getTags().stream().map(TagEntity::getName).collect(Collectors.toList()));
        response.setSubmissionCount(entity.getSubmissionCount());
        response.setAcceptedCount(entity.getAcceptedCount());
        response.setPassRate(calculatePassRate(entity.getSubmissionCount(), entity.getAcceptedCount()));
        return response;
    }
    // 详情响应包含题面、样例、统计和测试用例，供管理端编辑复用。

    private ProblemDetailResponse toDetail(ProblemEntity entity) {
        ProblemDetailResponse response = new ProblemDetailResponse();
        response.setId(entity.getId());
        response.setTitle(entity.getTitle());
        response.setDescription(entity.getDescription());
        response.setDifficulty(entity.getDifficulty().name());
        response.setTimeLimit(entity.getTimeLimit());
        response.setMemoryLimit(entity.getMemoryLimit());
        response.setInputDescription(entity.getInputDescription());
        response.setOutputDescription(entity.getOutputDescription());
        response.setSampleInput(entity.getSampleInput());
        response.setSampleOutput(entity.getSampleOutput());
        response.setTags(entity.getTags().stream().map(TagEntity::getName).collect(Collectors.toList()));
        response.setSubmissionCount(entity.getSubmissionCount());
        response.setAcceptedCount(entity.getAcceptedCount());
        response.setPassRate(calculatePassRate(entity.getSubmissionCount(), entity.getAcceptedCount()));
        response.setIsPublic(entity.getIsPublic());
        response.setTestCases(entity.getTestCases().stream().map(this::toTestCase).collect(Collectors.toList()));
        response.setCreatedAt(entity.getCreatedAt());
        response.setUpdatedAt(entity.getUpdatedAt());
        return response;
    }
    // 测试用例响应保留输入输出和样例标记，前端据此展示公开样例。

    private TestCaseResponse toTestCase(TestCaseEntity entity) {
        TestCaseResponse response = new TestCaseResponse();
        response.setId(entity.getId());
        response.setInput(entity.getInput());
        response.setOutput(entity.getOutput());
        response.setIsSample(entity.getIsSample());
        response.setScore(entity.getScore());
        return response;
    }
    // 写操作响应只返回定位和时间信息，前端再按 id 刷新详情。

    private ProblemMutationResponse toMutation(ProblemEntity entity) {
        ProblemMutationResponse response = new ProblemMutationResponse();
        response.setId(entity.getId());
        response.setTitle(entity.getTitle());
        response.setCreatedAt(entity.getCreatedAt());
        response.setUpdatedAt(entity.getUpdatedAt());
        return response;
    }
    // 通过率使用小数保存给前端格式化，避免后端重复处理百分号。

    private Double calculatePassRate(Integer submissionCount, Integer acceptedCount) {
        if (submissionCount == null || submissionCount == 0) {
            return 0D;
        }
        return acceptedCount == null ? 0D : acceptedCount.doubleValue() / submissionCount.doubleValue();
    }
}
