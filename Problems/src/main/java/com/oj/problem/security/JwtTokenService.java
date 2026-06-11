package com.oj.problem.security;

import com.fasterxml.jackson.core.type.TypeReference;
import com.fasterxml.jackson.databind.ObjectMapper;
import java.nio.charset.StandardCharsets;
import java.util.Base64;
import java.util.Map;
import javax.crypto.Mac;
import javax.crypto.spec.SecretKeySpec;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Service;

@Service
public class JwtTokenService {

    private final ObjectMapper objectMapper;
    // JWT 签名密钥，和用户模块保持一致才能解析身份。
    private final String secret;

    public JwtTokenService(ObjectMapper objectMapper,
                           @Value("${security.jwt.secret:oj-management-dev-secret}") String secret) {
        this.objectMapper = objectMapper;
        this.secret = secret;
    }

    public CurrentUser requireAdmin(String authorization) {
        CurrentUser user = parseAuthorization(authorization);
        if (!user.isAdmin()) {
            throw new IllegalArgumentException("权限不足");
        }
        return user;
    }

    public CurrentUser parseAuthorization(String authorization) {
        if (authorization == null || !authorization.startsWith("Bearer ")) {
            throw new IllegalArgumentException("未登录");
        }
        return parseToken(authorization.substring(7).trim());
    }

    private CurrentUser parseToken(String token) {
        try {
            // JWT 格式：header.payload.signature，我们只验证签名和解析 payload
            String[] parts = token.split("\\.");
            if (parts.length != 3) {
                throw new IllegalArgumentException("Token无效");
            }
            String signature = sign(parts[0] + "." + parts[1]);
            if (!signature.equals(parts[2])) {
                throw new IllegalArgumentException("Token无效");
            }
            // payload 是 base64 编码的 JSON，解析出 role 和 userId
            Map<String, Object> claims = objectMapper.readValue(
                    Base64.getUrlDecoder().decode(parts[1]),
                    new TypeReference<Map<String, Object>>() {
                    });
            Object role = claims.get("role");
            Object userId = claims.containsKey("user_id") ? claims.get("user_id") : claims.get("sub");
            if (role == null || userId == null) {
                throw new IllegalArgumentException("Token无效");
            }
            return new CurrentUser(Long.valueOf(String.valueOf(userId)), String.valueOf(role));
        } catch (IllegalArgumentException ex) {
            throw ex;
        } catch (Exception ex) {
            throw new IllegalArgumentException("Token无效", ex);
        }
    }

    private String sign(String content) throws Exception {
        Mac mac = Mac.getInstance("HmacSHA256");
        SecretKeySpec keySpec = new SecretKeySpec(secret.getBytes(StandardCharsets.UTF_8), "HmacSHA256");
        mac.init(keySpec);
        byte[] digest = mac.doFinal(content.getBytes(StandardCharsets.UTF_8));
        return Base64.getUrlEncoder().withoutPadding().encodeToString(digest);
    }
}
