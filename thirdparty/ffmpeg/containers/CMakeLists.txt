add_subdirectory(ogg)

# Propagate any meson option additions from codec subdirs to the parent
# (child CMakeLists use PARENT_SCOPE to set variables in this scope;
# forward them up to the top-level ffmpeg CMakeLists which called
# `add_subdirectory(codecs)` so Meson sees all requested options.)
if(DEFINED FFMPEG_MESON_OPTIONS)
	set(FFMPEG_MESON_OPTIONS ${FFMPEG_MESON_OPTIONS} PARENT_SCOPE)
endif()