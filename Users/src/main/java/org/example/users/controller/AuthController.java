package org.example.users.controller;

import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpSession;
import org.example.users.entity.User;
import org.example.users.service.UserService;
import org.example.users.utils.CredentialValidationUtils;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.servlet.mvc.support.RedirectAttributes;

@Controller
public class AuthController {
    private final UserService userService;

    public AuthController(UserService userService) {
        this.userService = userService;
    }

    // 展示欢迎界面
    @GetMapping({"", "index", "index.html"})
    public String showIndex() {
        return "index";
    }

    // 展示用户登录界面
    @GetMapping({"/login"})
    public String showLogin(HttpSession session, Model model) {
        // 检查 session 中的提示信息
        String homeMsg = (String) session.getAttribute("homeMsg");
        String loginMsg = (String) session.getAttribute("loginMsg");
        String registerMsg = (String) session.getAttribute("registerMsg");
        String sandboxMsg = (String) session.getAttribute("sandboxMsg");
        String profileMsg = (String) session.getAttribute("profileMsg");

        // 用户未登录
        if (homeMsg != null && homeMsg.equals("User not logged in")) {
                model.addAttribute("errorMsg", "用户未登录");
                session.removeAttribute("homeMsg");
        }
        // 用户未登录
        if (sandboxMsg != null && sandboxMsg.equals("User not logged in")) {
                model.addAttribute("errorMsg", "用户未登录");
                session.removeAttribute("sandboxMsg");
        }
        // 用户未登录
        if (profileMsg != null && profileMsg.equals("User not logged in")) {
                model.addAttribute("errorMsg", "用户未登录");
                session.removeAttribute("profileMsg");
        }
        // 用户名或密码错误
        if (loginMsg != null && loginMsg.equals("Incorrect username or password")) {
                model.addAttribute("errorMsg", "用户名或密码错误");
                session.removeAttribute("loginMsg");
        }
        // 注册成功
        if (registerMsg != null && registerMsg.equals("Registration successful")) {
                model.addAttribute("successMsg", "注册成功，请登录");
                session.removeAttribute("registerMsg");
        }

        return "login";
    }

    // 处理用户登录请求
    @PostMapping({"/login"})
    public String handleLogin(String username, String password, HttpSession session) {
        // 检验用户身份
        User user = userService.authenticate(username, password);
        if (user == null) {
            session.setAttribute("loginMsg", "Incorrect username or password");
            return "redirect:/login";
        }
        else {
            session.setAttribute("loginUser", username);
            return "redirect:/home";
        }
    }

    // 展示用户主页
    @GetMapping({"/home"})
    public String showHome(HttpSession session) {
        String username = (String) session.getAttribute("loginUser");
        // 无 Session 时重定向至登录界面
        if (username == null) {
            session.setAttribute("homeMsg", "User not logged in");
            return "redirect:/login";
        }
        return "home";
    }

    // 展示OI Wiki界面
    @GetMapping("/redirect-port8000")
    public String redirectToPort8000(HttpSession session, HttpServletRequest request) {
        String username = (String) session.getAttribute("loginUser");
        // 无 Session 时重定向至登录界面
        if (username == null) {
            session.setAttribute("homeMsg", "User not logged in");
            return "redirect:/login";
        }
        // 动态获取当前服务器地址，访问8000端口
        String serverAddr = request.getServerName();
        return "redirect:http://" + serverAddr + ":8000";
    }

    // 处理用户登出
    @GetMapping({"/logout"})
    public String handleLogout(HttpSession session, RedirectAttributes redirectAttributes) {
        // 销毁当前 session
        session.invalidate();

        // 将“退出登录成功”写入 Flash 中
        redirectAttributes.addFlashAttribute("successMsg", "退出登录成功");

        return "redirect:/login";
    }

    // 展示用户注册界面
    @GetMapping({"/register"})
    public String showRegister(HttpSession session, Model model) {
        // 检查 session 中的提示信息
        String registerMsg = (String) session.getAttribute("registerMsg");

        if (registerMsg != null) {
            // 用户名格式错误
            switch (registerMsg) {
                case "Invalid username format" -> {
                    model.addAttribute("errorMsg", "用户名格式错误，请不要尝试绕过前端 :)");
                    session.removeAttribute("registerMsg");
                }
                // 用户名已被占用
                case "Username is already taken" -> {
                    model.addAttribute("errorMsg", "用户名已被占用，请不要尝试绕过前端 :)");
                    session.removeAttribute("registerMsg");
                }
                // 密码格式错误
                case "Invalid password format" -> {
                    model.addAttribute("errorMsg", "密码格式错误，请不要尝试绕过前端 :)");
                    session.removeAttribute("registerMsg");
                }
                // 两次密码不一致
                case "Passwords do not match" -> {
                    model.addAttribute("errorMsg", "两次密码不一致，请不要尝试绕过前端 :)");
                    session.removeAttribute("registerMsg");
                }
            }
        }

        return "register";
    }

    // 处理用户注册请求
    @PostMapping({"/register"})
    public String handleRegister(String username, String password, String confirmPassword, HttpSession session) {
        // 后端再次检验用户名格式是否正确
        if (!CredentialValidationUtils.checkUsername(username)) {
            session.setAttribute("registerMsg", "Invalid username format");
            return "redirect:/register";
        }
        // 检验用户名是否已被占用
        if (userService.checkUsername(username)) {
            session.setAttribute("registerMsg", "Username is already taken");
            return "redirect:/register";
        }
        // 后端再次检验密码格式是否正确
        if (!CredentialValidationUtils.checkPassword(password)) {
            session.setAttribute("registerMsg", "Invalid password format");
            return "redirect:/register";
        }
        // 后端再次检验两次密码是否一致
        if (!password.equals(confirmPassword)) {
            session.setAttribute("registerMsg", "Passwords do not match");
            return "redirect:/register";
        }

        userService.insertUser(username, password);
        session.setAttribute("registerMsg", "Registration successful");
        return "redirect:/login";
    }
}