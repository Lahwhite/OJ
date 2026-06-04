package org.example.users.config;

import org.example.users.utils.IpAddressUtil;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.web.bind.annotation.ControllerAdvice;
import org.springframework.web.bind.annotation.ModelAttribute;

@ControllerAdvice
public class GlobalThymeleafConfig {

    @Value("${server.port}")
    private Integer currentServerPort;

    @Value("${service.problem.port}")
    private Integer problemPort;

    @Value("${service.discussion.port}")
    private Integer discussionPort;

    @Value("${service.rank.port}")
    private Integer rankPort;

    // 缓存本机IP，只获取一次
    private static volatile String cachedIp = null;

    private static String getCachedIp() {
        if (cachedIp == null) {
            synchronized (GlobalThymeleafConfig.class) {
                if (cachedIp == null) {
                    cachedIp = IpAddressUtil.getLocalIpv4();
                }
            }
        }
        return cachedIp;
    }

    @ModelAttribute("globalIp")
    public String getGlobalIp() {
        return getCachedIp();
    }

    @ModelAttribute("problemPort")
    public Integer getProblemPort() {
        return problemPort;
    }

    @ModelAttribute("discussionPort")
    public Integer getDiscussionPort() {
        return discussionPort;
    }

    @ModelAttribute("rankPort")
    public Integer getRankPort() {
        return rankPort;
    }

    @ModelAttribute("currentPort")
    public Integer getCurrentPort() {
        return currentServerPort;
    }
}