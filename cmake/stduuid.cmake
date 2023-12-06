include(FetchContent)

FetchContent_Declare(stduuid
		GIT_REPOSITORY https://github.com/mariusbancila/stduuid.git
		GIT_TAG tags/v1.2.3
		GIT_SHALLOW TRUE
		GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(stduuid)
