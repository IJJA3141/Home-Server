#include "logger.hpp"

void Logger::out(std::FILE* const _ostream)
{
  if (!_ostream) throw std::invalid_argument("");

  if (Logger::logger_->ostream_ && Logger::logger_->close_ostream_) std::fclose(Logger::logger_->ostream_);
  Logger::logger_->close_ostream_ = false;
  Logger::logger_->ostream_ = _ostream;
}

void Logger::out(const std::filesystem::path _path)
{
  std::FILE* const out = fopen(_path.c_str(), "a");
  if (!out) throw std::invalid_argument("");

  auto& logger = Logger::get();
  if (logger.ostream_ && logger.close_ostream_) std::fclose(logger.ostream_);
  Logger::logger_->ostream_ = out;
  Logger::logger_->close_ostream_ = true;
}

Logger::~Logger() noexcept
{
  // kills the worker and joins it with default jthread destructor
  // then close output file if needed
  running_ = false;
  smphr_.release();
  worker_.join();
  if (ostream_ && close_ostream_) std::fclose(ostream_);
}

const Logger::Instance Logger::get(const std::string& prefix)
{
  return {[=] -> const std::string { return prefix; }};
}

const Logger::Instance Logger::get(const std::string& prefix, Logger::ctx_f suffix)
{
  return {[=] -> const std::string { return prefix; }, suffix};
}

const Logger::Instance Logger::get(Logger::ctx_f prefix) { return {prefix}; }

const Logger::Instance Logger::get(ctx_f prefix, ctx_f suffix) { return {prefix, suffix}; }

std::unique_ptr<Logger> Logger::logger_ = std::make_unique<Logger>();
