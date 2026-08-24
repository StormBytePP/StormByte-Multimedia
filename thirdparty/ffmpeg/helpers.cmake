# Shared helpers for bundled FFmpeg + plugins (MSVC post-install normalize / strip)

set(FFMPEG_RENAME_MSVC_LIB
    "${CMAKE_CURRENT_LIST_DIR}/rename_msvc_lib.cmake"
    CACHE INTERNAL "Normalize MSVC static lib names (copy, idempotent)")

set(FFMPEG_STRIP_MSVC_RES
    "${CMAKE_CURRENT_LIST_DIR}/strip_msvc_res.cmake"
    CACHE INTERNAL "Strip embedded .res from MSVC static libs (idempotent)")