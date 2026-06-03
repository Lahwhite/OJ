#ifndef GEMINI_CLIENT_H
#define GEMINI_CLIENT_H

#include "discussion_service.h"

#include <string>
#include <vector>

struct DiscussionSummaryResult {
    std::string summary;
    std::string model;
};

class GeminiClient {
public:
    DiscussionSummaryResult summarizeDiscussion(const DiscussionTopic& topic,
                                                const std::vector<DiscussionComment>& comments) const;

    static bool configured();
    static std::string activeModel();

    static std::string buildSummaryPrompt(const DiscussionTopic& topic,
                                          const std::vector<DiscussionComment>& comments);
    static std::string extractTextFromResponse(const std::string& response_body);
};

#endif  // GEMINI_CLIENT_H
