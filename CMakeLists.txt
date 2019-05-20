cmake_minimum_required(VERSION 3.3)

include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.23.171.tar.gz"
    SHA1 "5d68bcca78eee347239ca5f4d34f4b6c12683154"
)

project(ffmpeg VERSION 4.1)

try_compile(
    ffmpeg_stdatomic_support
    "${CMAKE_CURRENT_BINARY_DIR}/ffmpeg_stdatomic_support/try_compile"
    "${CMAKE_CURRENT_LIST_DIR}/cmake/try_compile/stdatomic.c"
    OUTPUT_VARIABLE
    output
)

if(NOT ffmpeg_stdatomic_support)
  message(FATAL_ERROR "stdatomic is not supported: ${output}")
endif()

set(
    configure_script
    ${CMAKE_CURRENT_LIST_DIR}/configure
)

# Licensing options
option(FFMPEG_ENABLE_GPL "Allow use of GPL code" OFF)
if(FFMPEG_ENABLE_GPL)
  list(APPEND configure_script --enable-gpl)
else()
  list(APPEND configure_script --disable-gpl)
endif()

# Program options
option(FFMPEG_ENABLE_PROGRAMS "Build command line programs" OFF)
if(NOT FFMPEG_ENABLE_PROGRAMS)
  list(APPEND configure_script --disable-programs)
endif()

# Component options
option(FFMPEG_ENABLE_AVFILTER "Enable libavfilter build" OFF)
if(FFMPEG_ENABLE_AVFILTER)
   list(APPEND configure_script --enable-avfilter)
else()
   list(APPEND configure_script --disable-avfilter)
endif()

option(FFMPEG_ENABLE_SWSCALE "Enable libswscale build" OFF)
if(FFMPEG_ENABLE_SWSCALE)
  list(APPEND configure_script --enable-swscale)
else()
  list(APPEND configure_script --disable-swscale)
endif()

# Individual component options
set(FFMPEG_ENABLE_ENCODER "" CACHE STRING "List of encoders to enable")
foreach(x ${FFMPEG_ENABLE_ENCODER})
  list(APPEND configure_script --enable-encoder=${x})
endforeach()

set(FFMPEG_ENABLE_MUXER "" CACHE STRING "List of muxers to enable")
foreach(x ${FFMPEG_ENABLE_MUXER})
  list(APPEND configure_script --enable-muxer=${x})
endforeach()

set(FFMPEG_ENABLE_DEMUXER "matroska;mov" CACHE STRING "List of demuxers to enable")
foreach(x ${FFMPEG_ENABLE_DEMUXER})
  list(APPEND configure_script --enable-demuxer=${x})
endforeach()

set(FFMPEG_ENABLE_PROTOCOL "" CACHE STRING "List of protocols to enable")
foreach(x ${FFMPEG_ENABLE_PROTOCOL})
  list(APPEND configure_script --enable-protocol=${x})
endforeach()

set(FFMPEG_ENABLE_FILTER "" CACHE STRING "List of filters to enable")
foreach(x ${FFMPEG_ENABLE_FILTER})
  list(APPEND configure_script --enable-filter=${x})
endforeach()

# External library support
option(FFMPEG_ENABLE_LIBX264 "Enable H.264 encoding via x264" OFF)
if(FFMPEG_ENABLE_LIBX264)
  # Hunter will set PKG_CONFIG_LIBDIR environment variable, no other actions
  # needed to integrate x264. 'find_package' added for sanity check.
  hunter_add_package(x264)
  find_package(x264 CONFIG REQUIRED)
  list(APPEND configure_script --enable-libx264)
else()
  list(APPEND configure_script --disable-libx264)
endif()

option(FFMPEG_ENABLE_OPENCL "Enable OpenCL processing" OFF)
if(FFMPEG_ENABLE_OPENCL)
  list(APPEND configure_script --enable-opencl)
else()
  list(APPEND configure_script --disable-opencl)
endif()

set(ffmpeg_native_gen_dir "${CMAKE_CURRENT_BINARY_DIR}/ffmpeg_native_gen")
file(MAKE_DIRECTORY "${ffmpeg_native_gen_dir}")

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

    # Documentation options
    --disable-doc
    --disable-htmlpages
    --disable-manpages
    --disable-podpages
    --disable-txtpages

    # External library support:
    --disable-autodetect

    --disable-avdevice
    --enable-avcodec
    --enable-avformat
    --enable-swresample
    --disable-postproc
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

set(configure_human "${configure_script}")
string(REPLACE ";" " " configure_human "${configure_human}")
message("Run configure command:\n${configure_human}")

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

file(STRINGS "${ffmpeg_native_gen_dir}/ffbuild/config.mak" native_cfg)

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

# libavcodec/Makefile:OBJS-$(CONFIG_FRAME_THREAD_ENCODER) += frame_thread_encoder.o
if("CONFIG_FRAME_THREAD_ENCODER=yes" IN_LIST native_cfg)
  list(
      APPEND
      ffmpeg_codec_files
      libavcodec/frame_thread_encoder.c
  )
endif()

# libavcodec/Makefile:OBJS-$(CONFIG_BMP_ENCODER) += bmpenc.o
if("CONFIG_BMP_ENCODER=yes" IN_LIST native_cfg)
  list(
      APPEND
      ffmpeg_codec_files
      libavcodec/bmpenc.c
  )
endif()

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

# libavformat/Makefile:OBJS-$(CONFIG_IMAGE2_DEMUXER)             += img2dec.o img2.o
# libavformat/Makefile:OBJS-$(CONFIG_IMAGE2PIPE_DEMUXER)         += img2dec.o img2.o
# libavformat/Makefile:OBJS-$(CONFIG_IMAGE_BMP_PIPE_DEMUXER)     += img2dec.o img2.o
# libavformat/Makefile:OBJS-$(CONFIG_IMAGE_DDS_PIPE_DEMUXER)     += img2dec.o img2.o
# libavformat/Makefile:OBJS-$(CONFIG_IMAGE_DPX_PIPE_DEMUXER)     += img2dec.o img2.o
# libavformat/Makefile:OBJS-$(CONFIG_IMAGE_EXR_PIPE_DEMUXER)     += img2dec.o img2.o
# libavformat/Makefile:OBJS-$(CONFIG_IMAGE_J2K_PIPE_DEMUXER)     += img2dec.o img2.o
# libavformat/Makefile:OBJS-$(CONFIG_IMAGE_JPEG_PIPE_DEMUXER)    += img2dec.o img2.o
# libavformat/Makefile:OBJS-$(CONFIG_IMAGE_JPEGLS_PIPE_DEMUXER)  += img2dec.o img2.o
# libavformat/Makefile:OBJS-$(CONFIG_IMAGE_PAM_PIPE_DEMUXER)     += img2dec.o img2.o
# libavformat/Makefile:OBJS-$(CONFIG_IMAGE_PBM_PIPE_DEMUXER)     += img2dec.o img2.o
# libavformat/Makefile:OBJS-$(CONFIG_IMAGE_PCX_PIPE_DEMUXER)     += img2dec.o img2.o
# libavformat/Makefile:OBJS-$(CONFIG_IMAGE_PGMYUV_PIPE_DEMUXER)  += img2dec.o img2.o
# libavformat/Makefile:OBJS-$(CONFIG_IMAGE_PGM_PIPE_DEMUXER)     += img2dec.o img2.o
# libavformat/Makefile:OBJS-$(CONFIG_IMAGE_PICTOR_PIPE_DEMUXER)  += img2dec.o img2.o
# libavformat/Makefile:OBJS-$(CONFIG_IMAGE_PNG_PIPE_DEMUXER)     += img2dec.o img2.o
# libavformat/Makefile:OBJS-$(CONFIG_IMAGE_PPM_PIPE_DEMUXER)     += img2dec.o img2.o
# libavformat/Makefile:OBJS-$(CONFIG_IMAGE_PSD_PIPE_DEMUXER)     += img2dec.o img2.o
# libavformat/Makefile:OBJS-$(CONFIG_IMAGE_QDRAW_PIPE_DEMUXER)   += img2dec.o img2.o
# libavformat/Makefile:OBJS-$(CONFIG_IMAGE_SGI_PIPE_DEMUXER)     += img2dec.o img2.o
# libavformat/Makefile:OBJS-$(CONFIG_IMAGE_SVG_PIPE_DEMUXER)     += img2dec.o img2.o
# libavformat/Makefile:OBJS-$(CONFIG_IMAGE_SUNRAST_PIPE_DEMUXER) += img2dec.o img2.o
# libavformat/Makefile:OBJS-$(CONFIG_IMAGE_TIFF_PIPE_DEMUXER)    += img2dec.o img2.o
# libavformat/Makefile:OBJS-$(CONFIG_IMAGE_WEBP_PIPE_DEMUXER)    += img2dec.o img2.o
# libavformat/Makefile:OBJS-$(CONFIG_IMAGE_XPM_PIPE_DEMUXER)     += img2dec.o img2.o
# libavformat/Makefile:OBJS-$(CONFIG_IMAGE_XWD_PIPE_DEMUXER)     += img2dec.o img2.o

if(
    "CONFIG_IMAGE2_DEMUXER=yes" IN_LIST native_cfg OR
    "CONFIG_IMAGE2PIPE_DEMUXER=yes" IN_LIST native_cfg OR
    "CONFIG_IMAGE_BMP_PIPE_DEMUXER=yes" IN_LIST native_cfg OR
    "CONFIG_IMAGE_DDS_PIPE_DEMUXER=yes" IN_LIST native_cfg OR
    "CONFIG_IMAGE_DPX_PIPE_DEMUXER=yes" IN_LIST native_cfg OR
    "CONFIG_IMAGE_EXR_PIPE_DEMUXER=yes" IN_LIST native_cfg OR
    "CONFIG_IMAGE_J2K_PIPE_DEMUXER=yes" IN_LIST native_cfg OR
    "CONFIG_IMAGE_JPEG_PIPE_DEMUXER=yes" IN_LIST native_cfg OR
    "CONFIG_IMAGE_JPEGLS_PIPE_DEMUXER=yes" IN_LIST native_cfg OR
    "CONFIG_IMAGE_PAM_PIPE_DEMUXER=yes" IN_LIST native_cfg OR
    "CONFIG_IMAGE_PBM_PIPE_DEMUXER=yes" IN_LIST native_cfg OR
    "CONFIG_IMAGE_PCX_PIPE_DEMUXER=yes" IN_LIST native_cfg OR
    "CONFIG_IMAGE_PGMYUV_PIPE_DEMUXER=yes" IN_LIST native_cfg OR
    "CONFIG_IMAGE_PGM_PIPE_DEMUXER=yes" IN_LIST native_cfg OR
    "CONFIG_IMAGE_PICTOR_PIPE_DEMUXER=yes" IN_LIST native_cfg OR
    "CONFIG_IMAGE_PNG_PIPE_DEMUXER=yes" IN_LIST native_cfg OR
    "CONFIG_IMAGE_PPM_PIPE_DEMUXER=yes" IN_LIST native_cfg OR
    "CONFIG_IMAGE_PSD_PIPE_DEMUXER=yes" IN_LIST native_cfg OR
    "CONFIG_IMAGE_QDRAW_PIPE_DEMUXER=yes" IN_LIST native_cfg OR
    "CONFIG_IMAGE_SGI_PIPE_DEMUXER=yes" IN_LIST native_cfg OR
    "CONFIG_IMAGE_SVG_PIPE_DEMUXER=yes" IN_LIST native_cfg OR
    "CONFIG_IMAGE_SUNRAST_PIPE_DEMUXER=yes" IN_LIST native_cfg OR
    "CONFIG_IMAGE_TIFF_PIPE_DEMUXER=yes" IN_LIST native_cfg OR
    "CONFIG_IMAGE_WEBP_PIPE_DEMUXER=yes" IN_LIST native_cfg OR
    "CONFIG_IMAGE_XPM_PIPE_DEMUXER=yes" IN_LIST native_cfg OR
    "CONFIG_IMAGE_XWD_PIPE_DEMUXER=yes" IN_LIST native_cfg
)
  list(
      APPEND
      ffmpeg_format_files
      libavformat/img2dec.c
      libavformat/img2.c
  )
endif()

# libavformat/Makefile:OBJS-$(CONFIG_IMAGE2_MUXER)     += img2enc.o img2.o
# libavformat/Makefile:OBJS-$(CONFIG_IMAGE2PIPE_MUXER) += img2enc.o img2.o
if(
    "CONFIG_IMAGE2PIPE_MUXER=yes" IN_LIST native_cfg OR
    "CONFIG_IMAGE2_MUXER=yes" IN_LIST native_cfg
)
  list(
      APPEND
      ffmpeg_format_files
      libavformat/img2enc.c
      libavformat/img2.c
  )
endif()

# libavformat/Makefile:OBJS-$(CONFIG_FILE_PROTOCOL) += file.o
# libavformat/Makefile:OBJS-$(CONFIG_PIPE_PROTOCOL) += file.o
if(
    "CONFIG_FILE_PROTOCOL=yes" IN_LIST native_cfg OR
    "CONFIG_PIPE_PROTOCOL=yes" IN_LIST native_cfg
)
  list(
      APPEND
      ffmpeg_format_files
      libavformat/file.c
  )
endif()

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

if(FFMPEG_ENABLE_AVFILTER)
  set(
      avfilter_files
      libavfilter/af_aformat.c
      libavfilter/af_anull.c
      libavfilter/allfilters.c
      libavfilter/audio.c
      libavfilter/avfilter.c
      libavfilter/avfiltergraph.c
      libavfilter/buffersink.c
      libavfilter/buffersrc.c
      libavfilter/drawutils.c
      libavfilter/fifo.c
      libavfilter/formats.c
      libavfilter/framepool.c
      libavfilter/framequeue.c
      libavfilter/framesync.c
      libavfilter/graphdump.c
      libavfilter/graphparser.c
      libavfilter/pthread.c
      libavfilter/transform.c
      libavfilter/trim.c
      libavfilter/vf_format.c
      libavfilter/vf_null.c
      libavfilter/vf_overlay.c
      libavfilter/video.c
  )

  # libavfilter/Makefile:OBJS-$(CONFIG_SCALE2REF_FILTER) += vf_scale.o scale.o
  # libavfilter/Makefile:OBJS-$(CONFIG_SCALE_FILTER) += vf_scale.o scale.o
  if(
      "CONFIG_SCALE_FILTER=yes" IN_LIST native_cfg OR
      "CONFIG_SCALE2REF_FILTER=yes" IN_LIST native_cfg
  )
    list(
        APPEND
        avfilter_files
        libavfilter/vf_scale.c
        libavfilter/scale.c
    )
  endif()

  add_library(avfilter ${avfilter_files})

  list(APPEND all_targets avfilter)
endif()

if(FFMPEG_ENABLE_PROGRAMS)
  add_executable(
      ffmpeg
      fftools/ffmpeg_opt.c
      fftools/ffmpeg_filter.c
      fftools/ffmpeg_hw.c
      fftools/cmdutils.c
      fftools/ffmpeg.c
  )

  target_link_libraries(ffmpeg PRIVATE ${all_targets})
  list(APPEND all_targets ffmpeg)

  include(CheckLibraryExists)
  check_library_exists(m "pow" "" __math_system_library)
  if(__math_system_library)
    target_link_libraries(ffmpeg PRIVATE m)
  endif()

  find_package(Threads REQUIRED)
  target_link_libraries(ffmpeg PRIVATE Threads::Threads)
endif()

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
