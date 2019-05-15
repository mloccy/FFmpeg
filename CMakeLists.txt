cmake_minimum_required(VERSION 3.2)

include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.23.155.tar.gz"
    SHA1 "6421d63cc4fd78b8622e863c6b1dcabce98e1216"
)

project(ffmpeg VERSION 4.1)

set(ffmpeg_native_gen_dir "${CMAKE_CURRENT_BINARY_DIR}/ffmpeg_native_gen")
file(MAKE_DIRECTORY "${ffmpeg_native_gen_dir}")

set(
    configure_script
    ${CMAKE_CURRENT_LIST_DIR}/configure
)

if(BUILD_SHARED_LIBS)
  list(
      APPEND
      configure_script
      --disable-static
      --enable-shared
  )
else()
  list(
      APPEND
      configure_script
      --disable-shared
      --enable-static
  )
endif()

list(
    APPEND
    configure_script
    --enable-cross-compile
    --fatal-warnings
    --disable-programs
    --disable-doc
    --disable-avdevice
    --enable-avcodec
    --enable-avformat
    --enable-swresample
    --disable-swscale
    --disable-postproc
    --disable-avfilter
    --disable-avresample
    --disable-network
    --disable-everything
    --disable-bzlib
    --disable-iconv
    --disable-libxcb
    --disable-libxcb-shm
    --disable-libxcb-xfixes
    --disable-libxcb-shape
    --disable-lzma
    --disable-schannel
    --disable-sdl2
    --disable-securetransport
    --disable-xlib
    --disable-asm
    --enable-demuxer=matroska
    --enable-demuxer=mov
    --enable-parser=h264
    --enable-parser=hevc
    --enable-parser=vp8
    --enable-parser=vp9
    --enable-parser=aac
    --enable-parser=aac_latm
    --enable-parser=ac3
    --enable-parser=opus
    --enable-parser=vorbis
    --enable-decoder=h264
    --enable-decoder=hevc
    --enable-decoder=vp8
    --enable-decoder=vp9
    --enable-decoder=aac
    --enable-decoder=aac_latm
    --enable-decoder=ac3
    --enable-decoder=opus
    --enable-decoder=vorbis
    --enable-decoder=h264_cuvid
    --enable-decoder=hevc_cuvid
    --enable-decoder=vp8_cuvid
    --enable-decoder=vp9_cuvid
    --enable-decoder=aac_at
    --enable-decoder=ac3_at
    --enable-hwaccel=h264_nvdec
    --enable-hwaccel=h264_videotoolbox
    --enable-hwaccel=hevc_nvdec
    --enable-hwaccel=hevc_videotoolbox
    --enable-hwaccel=vp8_nvdec
    --enable-hwaccel=vp9_nvdec
    --enable-parser=mpeg4video
    --disable-optimizations
    --enable-debug
    --cc=${CMAKE_C_COMPILER}
    --cxx=${CMAKE_CXX_COMPILER}
)

# From Hunter (?)
list(
    APPEND
    configure_script
    --disable-zlib
)

execute_process(
    COMMAND
    ${configure_script}
    RESULT_VARIABLE
    result
    WORKING_DIRECTORY
    ${ffmpeg_native_gen_dir}
)

if(NOT result EQUAL "0")
  message(FATAL_ERROR "Configure failed: ${result}")
endif()


# Fixup CFLAGS and CXXFLAGS from autoconf output
option(FFMPEG_AUTOCONF_FLAGS "Grep flags from autoconf" OFF)

if(FFMPEG_AUTOCONF_FLAGS)
  file(READ "${ffmpeg_native_gen_dir}/ffbuild/config.mak" file_content)
  foreach(flag CFLAGS CXXFLAGS CPPFLAGS)
    string(REGEX MATCH "[\r|\n]${flag}=([^\r\n]+)[\r\n]" _ "${file_content}")
    if(NOT ${CMAKE_MATCH_COUNT} EQUAL 1)
      message(FATAL_ERROR "Could not find generated ${flag}")
    endif()
    set(${flag} "${CMAKE_MATCH_1}")
    string(REPLACE "$(SRC_PATH)" "${CMAKE_CURRENT_LIST_DIR}" ${flag} "${${flag}}")
  endforeach()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXXFLGAS} ${CPPFLAGS}")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CFLAGS} ${CPPFLAGS}")
endif()

# BUILD.gn
# config("external_config")

set(
    ffmpeg_codec_files
    libavcodec/ituh263dec.c
    libavcodec/h264_parse.c
    libavcodec/mediacodec.c
    libavcodec/mdct_fixed_32.c
    libavcodec/h264_cavlc.c
    libavcodec/opus_pvq.c
    libavcodec/vp9dsp_10bpp.c
    libavcodec/hevc_parser.c
    libavcodec/mdct_float.c
    libavcodec/vp8_parser.c
    libavcodec/h264_picture.c
    libavcodec/bitstream_filter.c
    libavcodec/hevc_filter.c
    libavcodec/h264_slice.c
    libavcodec/golomb.c
    libavcodec/faanidct.c
    libavcodec/avdct.c
    libavcodec/h264qpel.c
    libavcodec/jrevdct.c
    libavcodec/ac3tab.c
    libavcodec/mpegvideodata.c
    libavcodec/h264_mb.c
    libavcodec/decode.c
    libavcodec/h263dec.c
    libavcodec/vp9data.c
    libavcodec/vp9.c
    libavcodec/h263data.c
    libavcodec/h264dsp.c
    libavcodec/avpacket.c
    libavcodec/encode.c
    libavcodec/hevcdsp.c
    libavcodec/xiph.c
    libavcodec/hevc_mvs.c
    libavcodec/adts_parser.c
    libavcodec/vorbis_parser.c
    libavcodec/hpeldsp.c
    libavcodec/h264idct.c
    libavcodec/opus_parser.c
    libavcodec/h264pred.c
    libavcodec/idctdsp.c
    libavcodec/vp9_superframe_split_bsf.c
    libavcodec/aacsbr.c
    libavcodec/opusdec.c
    libavcodec/h263dsp.c
    libavcodec/mpeg12framerate.c
    libavcodec/h263.c
    libavcodec/bsf.c
    libavcodec/pthread_frame.c
    libavcodec/mpeg4videodec.c
    libavcodec/h264_parser.c
    libavcodec/mdct15.c
    libavcodec/h264_refs.c
    libavcodec/opus_celt.c
    libavcodec/hevc_sei.c
    libavcodec/kbdwin.c
    libavcodec/codec_desc.c
    libavcodec/hevc_parse.c
    libavcodec/error_resilience.c
    libavcodec/vp9block.c
    libavcodec/h264_cabac.c
    libavcodec/cbrt_data.c
    libavcodec/qsv_api.c
    libavcodec/vp9recon.c
    libavcodec/mathtables.c
    libavcodec/vp9dsp.c
    libavcodec/ac3dec_float.c
    libavcodec/sinewin_fixed.c
    libavcodec/ac3.c
    libavcodec/blockdsp.c
    libavcodec/mpegutils.c
    libavcodec/hevc_cabac.c
    libavcodec/bitstream_filters.c
    libavcodec/fft_init_table.c
    libavcodec/aactab.c
    libavcodec/startcode.c
    libavcodec/h264data.c
    libavcodec/cabac.c
    libavcodec/vp8.c
    libavcodec/aacps_float.c
    libavcodec/bswapdsp.c
    libavcodec/latm_parser.c
    libavcodec/fft_fixed_32.c
    libavcodec/h264dec.c
    libavcodec/opus_silk.c
    libavcodec/opus_rc.c
    libavcodec/fft_fixed.c
    libavcodec/jfdctint.c
    libavcodec/vorbis_data.c
    libavcodec/pthread_slice.c
    libavcodec/vp9dsp_8bpp.c
    libavcodec/opustab.c
    libavcodec/imgconvert.c
    libavcodec/vp9mvs.c
    libavcodec/raw.c
    libavcodec/hevc_refs.c
    libavcodec/faandct.c
    libavcodec/fdctdsp.c
    libavcodec/hevc_ps.c
    libavcodec/mpeg_er.c
    libavcodec/h2645_parse.c
    libavcodec/pthread.c
    libavcodec/hevc_data.c
    libavcodec/mjpegenc_huffman.c
    libavcodec/jni.c
    libavcodec/fmtconvert.c
    libavcodec/aac_ac3_parser.c
    libavcodec/dirac.c
    libavcodec/parsers.c
    libavcodec/h264chroma.c
    libavcodec/mpeg4video_parser.c
    libavcodec/vp9lpf.c
    libavcodec/bitstream.c
    libavcodec/qpeldsp.c
    libavcodec/mpegvideodsp.c
    libavcodec/aac_parser.c
    libavcodec/aacpsdsp_float.c
    libavcodec/h264_sei.c
    libavcodec/mpeg4video.c
    libavcodec/hevcdec.c
    libavcodec/mpegaudiodata.c
    libavcodec/vorbisdec.c
    libavcodec/vorbis.c
    libavcodec/vp8dsp.c
    libavcodec/opus.c
    libavcodec/vorbisdsp.c
    libavcodec/vp9_parser.c
    libavcodec/avfft.c
    libavcodec/me_cmp.c
    libavcodec/jfdctfst.c
    libavcodec/mdct_fixed.c
    libavcodec/mpegvideo.c
    libavcodec/pixblockdsp.c
    libavcodec/dv_profile.c
    libavcodec/sbrdsp.c
    libavcodec/h264_loopfilter.c
    libavcodec/ac3dec_data.c
    libavcodec/options.c
    libavcodec/mpegpicture.c
    libavcodec/sinewin.c
    libavcodec/d3d11va.c
    libavcodec/videodsp.c
    libavcodec/profiles.c
    libavcodec/simple_idct.c
    libavcodec/allcodecs.c
    libavcodec/aacdec.c
    libavcodec/avpicture.c
    libavcodec/mpeg4audio.c
    libavcodec/vp56rac.c
    libavcodec/h264_ps.c
    libavcodec/hevcpred.c
    libavcodec/utils.c
    libavcodec/ac3_parser.c
    libavcodec/h264_direct.c
    libavcodec/ac3dsp.c
    libavcodec/mpegvideo_motion.c
    libavcodec/rl.c
    libavcodec/adts_header.c
    libavcodec/fft_float.c
    libavcodec/vp9dsp_12bpp.c
    libavcodec/null_bsf.c
    libavcodec/parser.c
    libavcodec/vp9prob.c
)

if(APPLE)
  # config.h:#define CONFIG_AAC_ADTSTOASC_BSF 1
  list(
      APPEND
      ffmpeg_codec_files
      libavcodec/aac_adtstoasc_bsf.c
      libavcodec/mpeg4audio.c
  )

  # config.h:#define CONFIG_AAC_AT_DECODER 1
  # config.h:#define CONFIG_AC3_AT_DECODER 1
  list(
      APPEND
      ffmpeg_codec_files
      libavcodec/audiotoolboxdec.c
  )

  # config.h:#define CONFIG_H264_VIDEOTOOLBOX_HWACCEL 1
  # config.h:#define CONFIG_HEVC_VIDEOTOOLBOX_HWACCEL 1
  list(
      APPEND
      ffmpeg_codec_files
      libavcodec/videotoolbox.c
  )
else()
  # config.h:#define CONFIG_V4L2_M2M 0
  list(
      APPEND
      ffmpeg_codec_files
      libavcodec/v4l2_m2m.c
      libavcodec/v4l2_context.c
      libavcodec/v4l2_buffers.c
      libavcodec/v4l2_fmt.c
  )
endif()

add_library(avcodec ${ffmpeg_codec_files})

set(
    ffmpeg_format_files
    libavformat/mux.c
    libavformat/riff.c
    libavformat/isom.c
    libavformat/dump.c
    libavformat/avio.c
    libavformat/mov_esds.c
    libavformat/flac_picture.c
    libavformat/rmsipr.c
    libavformat/allformats.c
    libavformat/sdp.c
    libavformat/id3v2.c
    libavformat/format.c
    libavformat/cutils.c
    libavformat/aviobuf.c
    libavformat/options.c
    libavformat/url.c
    libavformat/mov.c
    libavformat/metadata.c
    libavformat/vorbiscomment.c
    libavformat/id3v1.c
    libavformat/matroska.c
    libavformat/replaygain.c
    libavformat/os_support.c
    libavformat/utils.c
    libavformat/matroskadec.c
    libavformat/protocols.c
    libavformat/mov_chan.c
    libavformat/riffdec.c
    libavformat/oggparsevorbis.c
    libavformat/qtpalette.c
)

add_library(avformat ${ffmpeg_format_files})
target_link_libraries(avformat PUBLIC avcodec)

set(
    ffmpeg_util_files
    libavutil/error.c
    libavutil/aes_ctr.c
    libavutil/crc.c
    libavutil/color_utils.c
    libavutil/mem.c
    libavutil/intmath.c
    libavutil/mastering_display_metadata.c
    libavutil/time.c
    libavutil/pixelutils.c
    libavutil/utils.c
    libavutil/lls.c
    libavutil/hash.c
    libavutil/cpu.c
    libavutil/lzo.c
    libavutil/cast5.c
    libavutil/buffer.c
    libavutil/audio_fifo.c
    libavutil/log2_tab.c
    libavutil/base64.c
    libavutil/file_open.c
    libavutil/file.c
    libavutil/camellia.c
    libavutil/random_seed.c
    libavutil/tree.c
    libavutil/display.c
    libavutil/rc4.c
    libavutil/des.c
    libavutil/murmur3.c
    libavutil/hwcontext.c
    libavutil/channel_layout.c
    libavutil/twofish.c
    libavutil/xga_font_data.c
    libavutil/pixdesc.c
    libavutil/bprint.c
    libavutil/slicethread.c
    libavutil/sha.c
    libavutil/float_dsp.c
    libavutil/xtea.c
    libavutil/md5.c
    libavutil/samplefmt.c
    libavutil/frame.c
    libavutil/mathematics.c
    libavutil/aes.c
    libavutil/fixed_dsp.c
    libavutil/dict.c
    libavutil/blowfish.c
    libavutil/imgutils.c
    libavutil/rational.c
    libavutil/tea.c
    libavutil/opt.c
    libavutil/eval.c
    libavutil/ripemd.c
    libavutil/hmac.c
    libavutil/downmix_info.c
    libavutil/sha512.c
    libavutil/avstring.c
    libavutil/threadmessage.c
    libavutil/lfg.c
    libavutil/encryption_info.c
    libavutil/adler32.c
    libavutil/reverse.c
    libavutil/parseutils.c
    libavutil/fifo.c
    libavutil/integer.c
    libavutil/spherical.c
    libavutil/stereo3d.c
    libavutil/log.c
    libavutil/timecode.c
)

if(APPLE)
  list(
      APPEND
      ffmpeg_util_files
      libavutil/hwcontext_videotoolbox.c
  )
endif()

add_library(avutil ${ffmpeg_util_files})

set(
    ffmpeg_swresample_files
    libswresample/resample.c
    libswresample/audioconvert.c
    libswresample/options.c
    libswresample/resample_dsp.c
    libswresample/swresample.c
    libswresample/dither.c
    libswresample/swresample_frame.c
    libswresample/rematrix.c
)

add_library(swresample ${ffmpeg_swresample_files})

set(
    ffmpeg_swscale_files
    libswscale/utils.c
    libswscale/options.c
    libswscale/input.c
    libswscale/swscale.c
    libswscale/hscale.c
    libswscale/hscale_fast_bilinear.c
    libswscale/swscale_unscaled.c
    libswscale/gamma.c
    libswscale/yuv2rgb.c
    libswscale/alphablend.c
    libswscale/rgb2rgb.c
    libswscale/slice.c
    libswscale/output.c
    libswscale/vscale.c
)

add_library(swscale ${ffmpeg_swscale_files})

set(
    all_targets
    avcodec
    avformat
    avutil
    swresample
    swscale
)

foreach(x ${all_targets})
  target_compile_definitions(
      ${x}
      PRIVATE

      # BUILD.gn
      # config("internal_config")
      HAVE_AV_CONFIG_H
  )
  target_include_directories(
      ${x}
      PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/forced>
      $<BUILD_INTERFACE:${ffmpeg_native_gen_dir}>
  )

  if(APPLE)
    find_package(audiotoolbox REQUIRED)
    target_link_libraries(${x} PUBLIC audiotoolbox::audiotoolbox)

    find_package(corefoundation REQUIRED)
    target_link_libraries(${x} PUBLIC corefoundation::corefoundation)

    find_package(coremedia REQUIRED)
    target_link_libraries(${x} PUBLIC coremedia::coremedia)

    find_package(coregraphics REQUIRED)
    target_link_libraries(${x} PUBLIC coregraphics::coregraphics)

    find_package(corevideo REQUIRED)
    target_link_libraries(${x} PUBLIC corevideo::corevideo)

    find_package(videotoolbox REQUIRED)
    target_link_libraries(${x} PUBLIC videotoolbox::videotoolbox)
  endif()
endforeach()

### Installation (https://github.com/forexample/package-example) {

# Introduce variables:
# * CMAKE_INSTALL_LIBDIR
# * CMAKE_INSTALL_BINDIR
# * CMAKE_INSTALL_INCLUDEDIR
include(GNUInstallDirs)

set(config_install_dir "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(version_config "${generated_dir}/${PROJECT_NAME}ConfigVersion.cmake")
set(project_config "${generated_dir}/${PROJECT_NAME}Config.cmake")
set(TARGETS_EXPORT_NAME "${PROJECT_NAME}Targets")
set(NAMESPACE "${PROJECT_NAME}::")

include(CMakePackageConfigHelpers)

# Use:
# * PROJECT_VERSION
write_basic_package_version_file(
    "${version_config}" COMPATIBILITY SameMajorVersion
)

# Use:
# * TARGETS_EXPORT_NAME
# * PROJECT_NAME
configure_package_config_file(
    "cmake/template/Config.cmake.in"
    "${project_config}"
    INSTALL_DESTINATION "${config_install_dir}"
)

install(
    FILES "${project_config}" "${version_config}"
    DESTINATION "${config_install_dir}"
)

install(
    EXPORT "${TARGETS_EXPORT_NAME}"
    NAMESPACE "${NAMESPACE}"
    DESTINATION "${config_install_dir}"
)

### }

install(
    TARGETS ${all_targets}
    EXPORT "${TARGETS_EXPORT_NAME}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(
    DIRECTORY
    libavcodec
    libavformat
    libavutil
    libswresample
    libswscale
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING
    PATTERN "*.h"
)

install(
    FILES
    ${ffmpeg_native_gen_dir}/libavutil/avconfig.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libavutil
)

install(
    FILES ${ffmpeg_native_gen_dir}/config.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libavutil
)

install(
    FILES ${ffmpeg_native_gen_dir}/config.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libavformat
)
