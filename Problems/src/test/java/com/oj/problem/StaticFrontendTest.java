package com.oj.problem;

import static org.hamcrest.Matchers.containsString;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.content;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.forwardedUrl;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.test.context.ActiveProfiles;
import org.springframework.test.web.servlet.MockMvc;

@SpringBootTest(properties = "spring.sql.init.mode=never")
@AutoConfigureMockMvc
@ActiveProfiles("test")
class StaticFrontendTest {

    @Autowired
    private MockMvc mockMvc;

    @Test
    void rootShouldServeProblemManagementPage() throws Exception {
        mockMvc.perform(get("/"))
                .andExpect(status().isOk())
                .andExpect(forwardedUrl("index.html"));

        mockMvc.perform(get("/index.html"))
                .andExpect(status().isOk())
                .andExpect(content().string(containsString("OJ Problems")));
    }

    @Test
    void staticAssetsShouldBeAvailable() throws Exception {
        mockMvc.perform(get("/styles.css"))
                .andExpect(status().isOk())
                .andExpect(content().string(containsString(".problem-card")));

        mockMvc.perform(get("/app.js"))
                .andExpect(status().isOk())
                .andExpect(content().string(containsString("./v1/problems")));
    }
}
