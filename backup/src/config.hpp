#pragma once

// const auto CERT_PATH = std::filesystem::path("/home/alexe/tmp/cert.pem");
// const auto KEY_PATH = std::filesystem::path("/home/alexe/tmp/key.pem");
constexpr auto CERT_PATH = "/home/alexe/tmp/cert.pem";
constexpr auto KEY_PATH = "/home/alexe/tmp/key.pem";

#define MIHON_SYNC_PORT 23
#define AUTH_PORT       24
#define BUFFER_SIZE     4096
#define LOCAL_HOST      "127.0.0.1"
// #define REVERSE_PROXY_PORT "0.0.0.0"
// #define MIHON_IP           "0.0.0.0"
// #define AUTH_IP            "0.0.0.0"
#define REVERSE_PROXY_PORT LOCAL_HOST
#define MIHON_IP           LOCAL_HOST
#define AUTH_IP            LOCAL_HOST

#define REVERSE_PROXY_MAX_QUEUE          100
#define REVERSE_PROXY_EPOLL_SIZE         10
#define REVERSE_PROXY_CLIENT_BUFFER_SIZE BUFFER_SIZE

// http cookies
// #define HTTP_SESSION_ID       "__Host-Http-session-id"
#define HTTP_SESSION_ID "session-id" // http test  only
static_assert(__DEBUG, "/!\\");
#define HTTP_PREFIX           "x-"
#define HTTP_WILDCARD         HTTP_PREFIX "wildcard"
#define HTTP_CONNECTION_TYPE  HTTP_PREFIX "connection-type"
#define HTTP_CLIENT_IP        HTTP_PREFIX "client-ip"
#define HTTP_UUID             HTTP_PREFIX "uuid"
#define CONNECTION_TYPE_HTTP  "http"
#define CONNECTION_TYPE_HTTPS "https"
