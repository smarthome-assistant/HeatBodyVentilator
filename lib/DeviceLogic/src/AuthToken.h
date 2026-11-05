#pragma once
#include <stddef.h>
#include <string.h>

// An empty session means nobody is logged in, so nothing may match it.
// Compares every byte so the response time does not reveal how much of a guess was right.
inline bool tokenMatches(const char* provided, const char* session) {
    const size_t sessionLength = strlen(session);
    const size_t providedLength = strlen(provided);
    if (sessionLength == 0) return false;

    unsigned char difference = providedLength == sessionLength ? 0 : 1;
    for (size_t i = 0; i < sessionLength; i++) {
        const char other = i < providedLength ? provided[i] : '\0';
        difference |= static_cast<unsigned char>(session[i] ^ other);
    }
    return difference == 0;
}
