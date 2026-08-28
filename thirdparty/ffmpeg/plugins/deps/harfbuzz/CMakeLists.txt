set(HARFBUZZ_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(HARFBUZZ_COMPONENT "harfbuzz")

set(HARFBUZZ_OPTIONS
	"-Dtests=disabled"
	"-Ddocs=disabled"
	"-Dbenchmark=disabled"
	"-Dicu=disabled"
	"-Dglib=disabled"
	"-Dcairo=disabled"
	"-Dfreetype=enabled"
)

buildmaster_component(
	harfbuzz
	"${HARFBUZZ_COMPONENT}"
	"${HARFBUZZ_SRC_DIR}"
	"${HARFBUZZ_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"harfbuzz"
	"BACKEND=meson;GIT={RESET;PATCH=${CMAKE_CURRENT_LIST_DIR}/0001-harfbuz-fix-long-paths.patch;TITLE=harfbuzz long paths};RENAME"
)
buildmaster_link(harfbuzz freetype)
buildmaster_group_add(ff-deps harfbuzz)
register_dependency(harfbuzz)
