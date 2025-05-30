#pragma once
#include "token.hpp"
#include <fmt/color.h>
#include <fmt/format.h>
#include <vector>

class ErrorReporter
{
  public:
    enum Level
    {
        WARNING,
        ERROR,
        FATAL
    };

    struct Message
    {
        Level severity;
        std::string message;
        int line;
        Token token;
    };

    template <typename... Args>
    inline auto report(Level level, const std::string &message, Args... args) -> void
    {
        auto formatted = fmt::format(fmt::runtime(message), args...);
        if (level != WARNING)
            has_error_ = true;
        messages.push_back({level, formatted, 0});
    }

    template <typename... Args>
    inline auto report(Level level, int line, const std::string &message, Args... args) -> void
    {
        auto formatted = fmt::format(fmt::runtime(message), args...);
        if (level != WARNING)
            has_error_ = true;
        messages.push_back({level, formatted, line});
    }

    template <typename... Args>
    inline auto report(Level level, Token token, const std::string &message, Args... args) -> void
    {
        auto formatted = fmt::format(fmt::runtime(message), args...);
        if (level != WARNING)
            has_error_ = true;
        messages.push_back({level, formatted, token.line, token});
    }

    auto display(FILE *stream, bool include_warnings = true, bool include_errors = true,
                 bool include_fatal = true) const -> void
    {
        for (auto message : messages)
        {
            std::string line = "_";

            // The message has line info
            if (message.line != 0)
                line = std::to_string(message.line);

            auto color = fmt::color::white;
            bool should_print = false;

            switch (message.severity)
            {
            case Level::WARNING:
                if (!include_warnings)
                    break;
                color = fmt::color::yellow;
                should_print = true;
                break;
            case Level::ERROR:
                if (!include_errors)
                    break;
                color = fmt::color::red;
                should_print = true;
                break;
            case Level::FATAL:
                if (!include_fatal)
                    break;
                color = fmt::color::magenta;
                should_print = true;
                break;
            }
            if (should_print)
            {
                fmt::print(stream, fmt::fg(color), "[Line {}] {}\n", line, message.message);
            }
        }
    }

    auto has_messages() const -> bool { return !messages.empty(); }

    auto has_error() const -> bool { return has_error_; }

    auto clear() -> void
    {
        messages.clear();
        has_error_ = false;
    }

  private:
    bool has_error_ = false;
    std::vector<Message> messages;
};