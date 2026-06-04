package org.example.users.config;

import org.example.users.utils.IpAddressUtil;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.web.bind.annotation.ControllerAdvice;
import org.springframework.web.bind.annotation.ModelAttribute;

/**
 * Thymeleaf全局变量注入：页面直接 ${globalIp} ${discussionPort} ...
 */
@ControllerAdvice // 全局生效，所有Controller的请求自动注入属性
public class GlobalThymeleafConfig {
    // 从配置文件读取端口
    @Value("${server.port}")
    private Integer currentServerPort; // 当前项目8081

    @Value("${service.discussion.port}")
    private Integer discussionPort; // 讨论区8079

    @Value("${service.rank.port}")
    private Integer rankPort; // 排行榜8092

    // 全局IP变量，Thymeleaf页面取值：${globalIp}
    @ModelAttribute("globalIp")
    public String getGlobalIp() {
        return IpAddressUtil.getLocalIpv4();
    }

    // 讨论区端口 ${discussionPort}
    @ModelAttribute("discussionPort")
    public Integer getDiscussionPort() {
        return discussionPort;
    }

    // 排行榜端口 ${rankPort}
    @ModelAttribute("rankPort")
    public Integer getRankPort() {
        return rankPort;
    }

    // 当前服务端口 ${currentPort}（return_url用到的8081）
    @ModelAttribute("currentPort")
    public Integer getCurrentPort() {
        return currentServerPort;
    }
}