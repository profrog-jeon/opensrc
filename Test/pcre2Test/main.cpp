#include "pch.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    pcre2_code* re;
    std::tstring pattern = TEXT("[0-9]+");   // 숫자 패턴
    std::tstring subject = TEXT("Hello123World456");
    int errornumber;
    PCRE2_SIZE erroroffset;
    pcre2_match_data* match_data;
    int rc;

    // 정규식 컴파일
    re = pcre2_compile(
        (PCRE2_SPTR)pattern.c_str(),               // 패턴
        PCRE2_ZERO_TERMINATED, // 널 종료 문자열
        0,                     // 옵션
        &errornumber,          // 에러 코드
        &erroroffset,          // 에러 위치
        NULL                   // 기본 컴파일 context
    );

    if (re == NULL) {
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(errornumber, buffer, sizeof(buffer));
        printf("PCRE2 compilation failed at offset %d: %s\n", (int)erroroffset, buffer);
        return 1;
    }

    // 매칭 데이터 생성
    match_data = pcre2_match_data_create_from_pattern(re, NULL);

    // 문자열 매칭
    rc = pcre2_match(
        re,             // 컴파일된 정규식
        (PCRE2_SPTR)subject.c_str(),        // 대상 문자열
        subject.length(), // 길이
        0,              // 시작 위치
        0,              // 옵션
        match_data,     // 매칭 결과 저장
        NULL            // 기본 match context
    );

    if (rc < 0) {
        printf("No match found.\n");
    }
    else {
        PCRE2_SIZE* ovector = pcre2_get_ovector_pointer(match_data);
        tprintf(TEXT("Match found: %.*s\n"),
            (int)(ovector[1] - ovector[0]),
            subject.c_str() + ovector[0]);
    }

    // 메모리 해제
    pcre2_match_data_free(match_data);
    pcre2_code_free(re);

    return 0;
}