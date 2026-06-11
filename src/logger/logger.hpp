#pragma once

#include <cstdio>
#include <filesystem>
#include <format>
#include <functional>
#include <memory>
#include <mutex>
#include <print>
#include <queue>
#include <semaphore>
#include <string>
#include <thread>
#include <utility>

#define ESC "\x1b["
#define RST "0m"

#define BLACK  "30m"
#define RED    "31m"
#define GREEN  "32m"
#define YELLOW "33m"
#define BLUE   "34m"
#define PURPLE "35m"
#define CYAN   "36m"
#define WHITE  "37m"

#define HBLACK  "90m"
#define HRED    "91m"
#define HGREEN  "92m"
#define HYELLOW "93m"
#define HBLUE   "94m"
#define HPURPLE "95m"
#define HCYAN   "96m"
#define HWHITE  "97m"

/**
 * @brief Thread-safe asynchronous logger.
 *
 * Logger provides formatted logging using std::format and writes
 * log messages from a dedicated background thread.
 */
struct Logger
{
private:
  using ctx_f = std::function<const std::string()>;

public:
  enum struct Level
  {
    INFO,
    WARN,
    ERROR,
    CRIT,
    DEBUG,
  };

  /* Converts a log level to a printable string. */
  static inline std::string ltos(const Level lvl)
  {
    switch (lvl)
    { // clang-format off
    case Logger::Level::INFO:  return "["            "info"          "]";
    case Logger::Level::DEBUG: return "[" ESC GREEN  "debug" ESC RST "]";
    case Logger::Level::WARN:  return "[" ESC YELLOW "warn"  ESC RST "]";
    case Logger::Level::ERROR: return "[" ESC RED    "error" ESC RST "]";
    case Logger::Level::CRIT:  return "[" ESC HRED   "crit"  ESC RST "]";
    default: std::unreachable();
    } // clang-format on
  }

  /**
   * @brief Sets the logger output stream.
   *
   * Ownership remains with the caller.
   *
   * @param ostream Output FILE.
   * @throws std::invalid_argument If output is nullptr.
   */
  static void out(std::FILE* const ostream);

  /**
   * @brief Opens a file and redirects logger output.
   *
   * The file is opened in append mode.
   * The logger assumes ownership and closes the file during destruction.
   *
   * @param path Output file path.
   * @throws std::invalid_argument If the file cannot be opened.
   */
  static void out(const std::filesystem::path path);

  static void level(const Level level) { Logger::logger_->level_ = level; }

  // destructor handels file closing and worker joining
  ~Logger() noexcept;

  template <typename... Args> static void log(std::format_string<Args...> fmt, Args&&... args)
  {
    Logger::logger_->send(std::format(fmt, std::forward<Args>(args)...));
  }

  template <Level lvl, typename... Args> static void log(std::format_string<Args...> fmt, Args&&... args)
  {
    if constexpr (lvl == Level::DEBUG && !__DEBUG) return;
    if (Logger::logger_->level_ <= lvl)
      Logger::logger_->send(ltos(lvl) + "\t" + std::format(fmt, std::forward<Args>(args)...));
  }

  template <typename... Args> static void debug(std::format_string<Args...> fmt, Args&&... args)
  {
    log<Level::DEBUG>(fmt, std::forward<Args>(args)...);
  }

  template <typename... Args> static void info(std::format_string<Args...> fmt, Args&&... args)
  {
    log<Level::INFO>(fmt, std::forward<Args>(args)...);
  }

  template <typename... Args> static void warn(std::format_string<Args...> fmt, Args&&... args)
  {
    log<Level::WARN>(fmt, std::forward<Args>(args)...);
  }

  template <typename... Args> static void error(std::format_string<Args...> fmt, Args&&... args)
  {
    log<Level::ERROR>(fmt, std::forward<Args>(args)...);
  }

  template <typename... Args> static void crit(std::format_string<Args...> fmt, Args&&... args)
  {
    log<Level::CRIT>(fmt, std::forward<Args>(args)...);
  }

private:
  // logger instance with context
  struct Instance
  {
    ctx_f prefix, suffix;

    template <typename... Args> void log(std::format_string<Args...> fmt, Args&&... args) const
    {
      std::string str;
      if (prefix) str += "(" + prefix() + ")\t";
      str += std::format(fmt, std::forward<Args>(args)...);
      if (suffix) str += "\t" + suffix();
      Logger::logger_->send(std::move(str));
    }

    template <Level level, typename... Args> void log(std::format_string<Args...> fmt, Args&&... args) const
    {
      if constexpr (level == Level::DEBUG && !__DEBUG) return;
      if (Logger::logger_->level_ <= level)
      {
        std::string str;
        if (prefix) str += "(" + prefix() + ")  ";
        str += std::format(fmt, std::forward<Args>(args)...);

        if constexpr (level == Level::ERROR || level == Level::CRIT)
          if (suffix) str += "\t" + suffix();

        Logger::logger_->send(ltos(level) + " " + str);
      }
    }

    template <typename... Args> void debug(std::format_string<Args...> fmt, Args&&... args) const
    {
      this->log<Level::DEBUG>(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args> void info(std::format_string<Args...> fmt, Args&&... args) const
    {
      this->log<Level::INFO>(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args> void warn(std::format_string<Args...> fmt, Args&&... args) const
    {
      this->log<Level::WARN>(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args> void error(std::format_string<Args...> fmt, Args&&... args) const
    {
      this->log<Level::ERROR>(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args> void crit(std::format_string<Args...> fmt, Args&&... args) const
    {
      this->log<Level::CRIT>(fmt, std::forward<Args>(args)...);
    }
  };

public:
  static inline const Logger& get() { return *logger_; }
  static const Instance get(const std::string& prefix);
  static const Instance get(const std::string& prefix, ctx_f suffix);
  static const Instance get(ctx_f prefix);
  static const Instance get(ctx_f prefix, ctx_f suffix);

private:
  static std::unique_ptr<Logger> logger_;

  std::counting_semaphore<> smphr_{0};
  std::queue<std::string> logs_{};
  std::mutex mtx_{};
  std::FILE* ostream_ = stdout;
  bool close_ostream_ = false;
  Level level_ = Level::INFO;
  std::atomic_bool running_ = true;

  // handels is in differante thread for thread safety and async io for speed
  std::jthread worker_{[&]() {
    std::string log;
    while (running_)
    {
      smphr_.acquire();

      {
        if (logs_.empty()) continue;
        std::lock_guard lock(mtx_);
        std::swap(log, logs_.front());
        logs_.pop();
      }

      std::println(ostream_, "{}", log);
    }
  }};

  // adds a log to logging queue
  void send(std::string&& log)
  {
    {
      std::lock_guard lock(mtx_);
      logs_.push(log);
    }

    smphr_.release();
  }
};
