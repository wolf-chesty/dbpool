INCLUDE (FetchContent)

FETCHCONTENT_DECLARE (date
        GIT_REPOSITORY https://github.com/HowardHinnant/date.git
        GIT_TAG tags/v3.0.1
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)

FETCHCONTENT_MAKEAVAILABLE (date)
