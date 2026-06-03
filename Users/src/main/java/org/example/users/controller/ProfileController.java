package org.example.users.controller;

import jakarta.servlet.http.HttpSession;
import lombok.extern.slf4j.Slf4j;
import org.example.users.entity.Profile;
import org.example.users.service.ProfileService;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.multipart.MultipartFile;

import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Set;
import java.util.UUID;

@Slf4j
@Controller
public class ProfileController {
    private final ProfileService profileService;
    private static final String UPLOAD_DIR = "uploads/avatars/";

    public ProfileController(ProfileService profileService) {
        this.profileService = profileService;
    }

    // 允许的图片格式
    private static final Set<String> ALLOWED_EXTENSIONS = Set.of("jpg", "jpeg", "png", "bmp", "gif");
    private static final Set<String> ALLOWED_MIME_TYPES = Set.of("image/jpeg", "image/png", "image/bmp", "image/gif");

    // 魔术数字映射
    private static final Map<String, byte[]> MAGIC_BYTES = new LinkedHashMap<>();
    static {
        MAGIC_BYTES.put("jpg",  new byte[]{(byte)0xFF, (byte)0xD8, (byte)0xFF});
        MAGIC_BYTES.put("png",  new byte[]{(byte)0x89, (byte)0x50, (byte)0x4E, (byte)0x47});
        MAGIC_BYTES.put("bmp",  new byte[]{(byte)0x42, (byte)0x4D});
        MAGIC_BYTES.put("gif",  new byte[]{(byte)0x47, (byte)0x49, (byte)0x46, (byte)0x38});
    }

    private boolean matchesMagic(byte[] header, byte[] magic) {
        if (header.length < magic.length) {
            return false;
        }
        for (int i=0; i<magic.length; i++) {
            if (header[i] != magic[i]) {
                return false;
            }
        }
        return true;
    }

    @GetMapping("/profile")
    public String showProfile(HttpSession session, Model model) {
        String username = (String) session.getAttribute("loginUser");
        // 无 Session 时重定向至登录界面
        if (username == null) {
            session.setAttribute("profileMsg", "User not logged in");
            return "redirect:/login";
        }

        String profileMsg = (String) session.getAttribute("profileMsg");
        if (profileMsg != null) {
            session.removeAttribute("profileMsg");
            switch (profileMsg) {
                case "File content is empty or invalid" -> model.addAttribute("errorMsg", "文件为空或已损坏");
                case "Invalid file type" -> model.addAttribute("errorMsg", "只允许上传 jpg、jpeg、png、bmp、gif 格式的图片");
                case "Avatar upload failed" -> model.addAttribute("errorMsg", "头像上传失败");
                case "Profile saved successfully" -> model.addAttribute("successMsg", "个人资料保存成功");
                case "Save failed" -> model.addAttribute("errorMsg", "保存失败，请重试");
            }
        }

        Profile profile = profileService.getProfileByUsername(username);
        if (profile == null) {
            model.addAttribute("nickname", "");
            model.addAttribute("signature", "");
            model.addAttribute("avatarUrl", "/images/default-avatar.png");
        }
        else {
            model.addAttribute("nickname", profile.getNickname() != null ? profile.getNickname() : "");
            model.addAttribute("signature", profile.getSignature() != null ? profile.getSignature() : "");
            model.addAttribute("avatarUrl", profile.getAvatar() != null ? profile.getAvatar() : "/images/default-avatar.png");
        }

        return "profile";
    }

    @PostMapping("/profile")
    public String handleProfile(@RequestParam(value = "nickname", required = false, defaultValue = "") String nickname,
                                @RequestParam(value = "signature", required = false, defaultValue = "") String signature,
                                @RequestParam(value = "avatar", required = false) MultipartFile avatarFile,
                                HttpSession session) {
        String username = (String) session.getAttribute("loginUser");
        // 无 Session 时重定向至登录界面
        if (username == null) {
            session.setAttribute("profileMsg", "User not logged in");
            return "redirect:/login";
        }

        // 头像上传处理
        String avatarPath = null;
        if (avatarFile != null && !avatarFile.isEmpty()) {
            String originalFilename = avatarFile.getOriginalFilename();
            String extension = "";

            // 提取扩展名
            if (originalFilename != null && originalFilename.contains(".")) {
                extension = originalFilename.substring(originalFilename.lastIndexOf(".") + 1).toLowerCase();
                if (!ALLOWED_EXTENSIONS.contains(extension)) {
                    session.setAttribute("profileMsg", "Invalid file type");
                    return "redirect:/profile";
                }
            }

            // 使用 MIME 类型进行初步过滤
            String contentType = avatarFile.getContentType();
            if (contentType == null || !ALLOWED_MIME_TYPES.contains(contentType)) {
                session.setAttribute("profileMsg", "Invalid file type");
                return "redirect:/profile";
            }

            // 使用 magic number 进行进一步过滤
            byte[] header = new byte[4];
            try (InputStream inputStream = avatarFile.getInputStream()) {
                int read = inputStream.read(header);
                if (read < 2) {
                    session.setAttribute("profileMsg", "File content is empty or invalid");
                    return "redirect:/profile";
                }
            } catch (IOException e) {
                session.setAttribute("profileMsg", "Avatar upload failed");
                return "redirect:/profile";
            }

            String detectedExt = null;
            for (Map.Entry<String, byte[]> entry : MAGIC_BYTES.entrySet()) {
                byte[] magic = entry.getValue();
                if (header.length >= magic.length && matchesMagic(header, magic)) {
                    // 取第一个匹配的，且 jpeg 被统一为 jpg
                    detectedExt = entry.getKey();
                    break;
                }
            }
            if (detectedExt == null) {
                session.setAttribute("profileMsg", "Invalid file type");
                return "redirect:/profile";
            }

            if (extension.isEmpty()) {
                extension = detectedExt;
            }
            else if (!extension.equals(detectedExt) && !(extension.equals("jpeg") && detectedExt.equals("jpg"))) {
                extension = detectedExt;
            }

            String newFileName = UUID.randomUUID() + "." + extension;
            try {
                // 创建存储目录
                Path uploadPath = Paths.get(UPLOAD_DIR);
                if (!Files.exists(uploadPath)) {
                    Files.createDirectories(uploadPath);
                }

                Path filePath = uploadPath.resolve(newFileName);
                try (InputStream fileInputStream = avatarFile.getInputStream()) {
                    Files.copy(fileInputStream, filePath, StandardCopyOption.REPLACE_EXISTING);
                }

                avatarPath = "/uploads/avatars/" + newFileName;
            } catch (IOException e) {
                session.setAttribute("profileMsg", "Avatar upload failed");
                return "redirect:/profile";
            }
        }

        // 旧头像路径
        String oldAvatarPath = null;
        if (avatarFile != null && !avatarFile.isEmpty()) {
            Profile oldProfile = profileService.getProfileByUsername(username);
            if (oldProfile != null) {
                oldAvatarPath = oldProfile.getAvatar();
            }
        }

        // 保存个人资料
        try {
            profileService.updateProfileByUsername(username, nickname, signature, avatarPath);
            session.setAttribute("profileMsg", "Profile saved successfully");
            if (oldAvatarPath != null && !oldAvatarPath.equals("/images/default-avatar.png")) {
                try {
                    Path oldFile = Paths.get(oldAvatarPath.startsWith("/") ? oldAvatarPath.substring(1) : oldAvatarPath);
                    Files.deleteIfExists(oldFile);
                } catch (IOException ignored) {
                    // 删除失败仅记录日志
                    log.error("删除旧头像失败");
                }
            }
        } catch (Exception e) {
            session.setAttribute("profileMsg", "Save failed");
        }

        return "redirect:/profile";
    }
}