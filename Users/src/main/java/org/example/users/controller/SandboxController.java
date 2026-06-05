package org.example.users.controller;

import lombok.extern.slf4j.Slf4j;
import org.example.users.dto.CodeRunRequest;
import org.example.users.dto.CodeRunResponse;
import org.example.users.enums.CodeLanguage;
import org.example.users.service.SandboxService;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestParam;

import jakarta.servlet.http.HttpSession;
import java.util.Arrays;
import java.util.List;
import java.util.Objects;
import java.util.stream.Collectors;

@Slf4j
@Controller
public class SandboxController {
    private final SandboxService sandboxService;

    public SandboxController(SandboxService sandboxService) {
        this.sandboxService = sandboxService;
    }

    // 提供编程语言列表
    private static final List<String> languages = Arrays.stream(CodeLanguage.values())
            .map(Enum::name)
            .map(String::toLowerCase)
            .collect(Collectors.toList());

    // 展示在线代码运行界面
    @GetMapping({"/sandbox"})
    public String showSandbox(HttpSession session, Model model) {
        String username = (String) session.getAttribute("loginUser");
        // 无 Session 时重定向至登录界面
        if (username == null) {
            session.setAttribute("sandboxMsg", "User not logged in");
            return "redirect:/login";
        }

        model.addAttribute("languages", languages);

        // 将用户输入的 code、language 还原
        String sandboxCode = (String) session.getAttribute("sandboxCode");
        if (sandboxCode != null) {
            model.addAttribute("code", sandboxCode);
            session.removeAttribute("sandboxCode");
        }

        String sandboxLanguage = (String) session.getAttribute("sandboxLanguage");
        if (sandboxLanguage != null) {
            model.addAttribute("selectedLanguage", sandboxLanguage);
            session.removeAttribute("sandboxLanguage");
        }

        // 检查 session 中的提示信息
        String sandboxMsg = (String) session.getAttribute("sandboxMsg");
        if (sandboxMsg != null) {
            session.removeAttribute("sandboxMsg");
            switch (sandboxMsg) {
                // 代码为空
                case "Empty code" -> model.addAttribute("errorMsg", "代码为空");
                // 未选择编程语言
                case "Language unselected" -> model.addAttribute("errorMsg", "未选择编程语言");
                // 运行成功
                case "success" -> {
                    String output = (String) session.getAttribute("output");
                    model.addAttribute("output", output);
                    session.removeAttribute("output");
                }
                // 系统异常
                case "System Error" -> model.addAttribute("errorMsg", "系统异常");
                // 运行出错
                case "failure" -> {
                    String output = (String) session.getAttribute("output");
                    if (output != null) {
                        model.addAttribute("output", output);
                    }
                    String errorMsg = (String) session.getAttribute("errorMsg");
                    model.addAttribute("errorMsg", Objects.requireNonNullElse(errorMsg, "未知错误"));
                    session.removeAttribute("output");
                    session.removeAttribute("errorMsg");
                }
            }
        }

        return "sandbox";
    }

    @PostMapping({"/sandbox"})
    public String handleSandbox(@RequestParam(required = false) String language,
                                @RequestParam(required = false) String code,
                                HttpSession session) {
        String username = (String) session.getAttribute("loginUser");
        // 无 Session 时重定向至登录界面
        if (username == null) {
            session.setAttribute("sandboxMsg", "User not logged in");
            return "redirect:/login";
        }

        // 将当前 code、language 存储下来，后续使用
        session.setAttribute("sandboxLanguage", language);
        session.setAttribute("sandboxCode", code);

        // 参数是否为空
        if (language == null || language.isBlank()) {
            session.setAttribute("sandboxMsg", "Language unselected");
            return "redirect:/sandbox";
        }
        if (code == null || code.isBlank()) {
            session.setAttribute("sandboxMsg", "Empty code");
            return "redirect:/sandbox";
        }

        // 构建请求并调用沙箱服务
        CodeRunRequest request = new CodeRunRequest();
        request.setCode(code);
        request.setLanguage(language);

        try {
            CodeRunResponse response = sandboxService.runCode(request);

            if (response.getSuccess()) {
                session.setAttribute("sandboxMsg", "success");
                session.setAttribute("output", response.getOutput());
            }
            else {
                session.setAttribute("sandboxMsg", "failure");
                if (response.getOutput() != null && !response.getOutput().isBlank()) {
                    session.setAttribute("output", response.getOutput());
                }
                if (response.getErrorMessage() != null && !response.getErrorMessage().isBlank()) {
                    session.setAttribute("errorMsg", response.getErrorMessage());
                }
            }
        } catch (Exception e) {
            log.error("沙箱运行出错", e);
            session.setAttribute("sandboxMsg", "System Error");
        }

        return "redirect:/sandbox";
    }
}