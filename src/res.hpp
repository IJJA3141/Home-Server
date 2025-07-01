#pragma once

#include "network/http.hpp"

const http::Response READ_ERROR(http::Protocol::HTTP_11, HTTP_INTERNAL_SERVER_ERROR, HTTP_MIME_PLAIN,
                                {
                                    {"Connection", "Close"},
                                },
                                "Failed to read request from client: Internal error.");
