package org.example.users.controller;

import org.springframework.http.*;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.ResponseBody;
import org.springframework.web.client.RestTemplate;

import jakarta.servlet.http.HttpSession;
import java.util.Map;

@Controller
public class RagController {

    // Python RAG 服务地址
    private static final String RAG_SERVICE_URL = "http://127.0.0.1:8001/api/rag/query";

    @PostMapping("/api/rag/query")
    @ResponseBody
    public ResponseEntity<Map<String, String>> ragQuery(@RequestBody Map<String, String> request,
                                                        HttpSession session) {
        // 检查登录状态
        String loginUser = (String) session.getAttribute("loginUser");
        if (loginUser == null) {
            return ResponseEntity.status(HttpStatus.UNAUTHORIZED)
                    .body(Map.of("answer", "请先登录后再使用问答功能"));
        }

        String question = request.get("question");
        if (question == null || question.trim().isEmpty()) {
            return ResponseEntity.badRequest()
                    .body(Map.of("answer", "问题不能为空"));
        }

        // 每次调用直接创建 RestTemplate（轻量，内部使用连接池默认配置）
        RestTemplate restTemplate = new RestTemplate();
        HttpHeaders headers = new HttpHeaders();
        headers.setContentType(MediaType.APPLICATION_JSON);
        HttpEntity<Map<String, String>> entity = new HttpEntity<>(request, headers);

        try {
            ResponseEntity<Map> response = restTemplate.postForEntity(RAG_SERVICE_URL, entity, Map.class);
            if (response.getStatusCode().is2xxSuccessful() && response.getBody() != null) {
                return ResponseEntity.ok(response.getBody());
            } else {
                return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR)
                        .body(Map.of("answer", "RAG 服务返回错误状态码"));
            }
        } catch (Exception e) {
            e.printStackTrace();
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR)
                    .body(Map.of("answer", "RAG 服务暂时不可用，请稍后重试。错误：" + e.getMessage()));
        }
    }
}