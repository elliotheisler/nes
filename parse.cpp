#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef enum { SUCCESS=1, FAIL=0 } ParseStateType;
typedef struct {
    ParseStateType type;
    char* state_or_msg;
    void* result;
} ParseState;

ParseState expect(char* state, char* expected_str) {
    size_t estr_len = strlen(expected_str);
    if (strncmp(state, expected_str, estr_len)) {
        return { FAIL, "failure in expect" };
    } else {
        return { SUCCESS, state + estr_len };
    }
}

void fail(const char * const msg) {
    dprintf(STDERR_FILENO, "parse fail: ");
    dprintf(STDERR_FILENO, msg);
    exit(EXIT_FAILURE);
}

