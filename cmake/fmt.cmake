INCLUDE (FetchContent)

FETCHCONTENT_DECLARE (fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG tags/10.1.1
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)

FETCHCONTENT_MAKEAVAILABLE (fmt)

