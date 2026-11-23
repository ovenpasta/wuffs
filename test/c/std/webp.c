// Copyright 2024 The Wuffs Authors.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// https://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or https://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.
//
// SPDX-License-Identifier: Apache-2.0 OR MIT

// ----------------

/*
This test program is typically run indirectly, by the "wuffs test" or "wuffs
bench" commands. These commands take an optional "-mimic" flag to check that
Wuffs' output mimics (i.e. exactly matches) other libraries' output, such as
giflib for GIF, libpng for PNG, etc.

To manually run this test:

for CC in clang gcc; do
  $CC -std=c99 -Wall -Werror webp.c && ./a.out
  rm -f a.out
done

Each edition should print "PASS", amongst other information, and exit(0).

Add the "wuffs mimic cflags" (everything after the colon below) to the C
compiler flags (after the .c file) to run the mimic tests.

To manually run the benchmarks, replace "-Wall -Werror" with "-O3" and replace
the first "./a.out" with "./a.out -bench". Combine these changes with the
"wuffs mimic cflags" to run the mimic benchmarks.
*/

// ¿ wuffs mimic cflags: -DWUFFS_MIMIC -lwebp

// Wuffs ships as a "single file C library" or "header file library" as per
// https://github.com/nothings/stb/blob/master/docs/stb_howto.txt
//
// To use that single file as a "foo.c"-like implementation, instead of a
// "foo.h"-like header, #define WUFFS_IMPLEMENTATION before #include'ing or
// compiling it.
#define WUFFS_IMPLEMENTATION

// Defining the WUFFS_CONFIG__MODULE* macros are optional, but it lets users of
// release/c/etc.c choose which parts of Wuffs to build. That file contains the
// entire Wuffs standard library, implementing a variety of codecs and file
// formats. Without this macro definition, an optimizing compiler or linker may
// very well discard Wuffs code for unused codecs, but listing the Wuffs
// modules we use makes that process explicit. Preprocessing means that such
// code simply isn't compiled.
#define WUFFS_CONFIG__MODULES
#define WUFFS_CONFIG__MODULE__BASE
#define WUFFS_CONFIG__MODULE__VP8
#define WUFFS_CONFIG__MODULE__WEBP

// If building this program in an environment that doesn't easily accommodate
// relative includes, you can use the script/inline-c-relative-includes.go
// program to generate a stand-alone C file.
#include "../../../release/c/wuffs-unsupported-snapshot.c"
#include "../testlib/testlib.c"
#ifdef WUFFS_MIMIC
#include "../mimiclib/webp.c"
#endif

static wuffs_webp__decoder g_webp_decoder;

// ---------------- WebP Tests

const char*  //
wuffs_webp_decode(uint64_t* n_bytes_out,
                  wuffs_base__io_buffer* dst,
                  uint32_t wuffs_initialize_flags,
                  wuffs_base__pixel_format pixfmt,
                  uint32_t* quirks_ptr,
                  size_t quirks_len,
                  wuffs_base__io_buffer* src) {
  wuffs_webp__decoder* dec = &g_webp_decoder;
  CHECK_STATUS("initialize",
               wuffs_webp__decoder__initialize(dec, sizeof *dec, WUFFS_VERSION,
                                               wuffs_initialize_flags));
  return do_run__wuffs_base__image_decoder(
      wuffs_webp__decoder__upcast_as__wuffs_base__image_decoder(dec),
      n_bytes_out, dst, pixfmt, quirks_ptr, quirks_len, src);
}

// --------

const char*  //
test_wuffs_webp_decode_frame_config() {
  CHECK_FOCUS(__func__);

  static const wuffs_base__rect_ie_u32 hippopotamus_want_bounds[] = {
      {0, 0, 36, 28},
  };
  static const uint64_t hippopotamus_want_io_ps[] = {
      0x001E,
  };
  static const uint64_t hippopotamus_want_durations[] = {
      0,
  };
  static const bool hippopotamus_want_overwrites[] = {
      false,
  };
  static const wuffs_base__rect_ie_u32 animated_red_blue_want_bounds[] = {
      {0, 0, 64, 48},
      {14, 30, 52, 40},
      {14, 0, 64, 40},
      {14, 0, 64, 40},
  };
  static const uint64_t animated_red_blue_want_io_ps[] = {
      0x002C,
      0x011C,
      0x017A,
      0x0252,
  };
  static const uint64_t animated_red_blue_want_durations[] = {
      100 * WUFFS_BASE__FLICKS_PER_MILLISECOND,
      200 * WUFFS_BASE__FLICKS_PER_MILLISECOND,
      300 * WUFFS_BASE__FLICKS_PER_MILLISECOND,
      400 * WUFFS_BASE__FLICKS_PER_MILLISECOND,
  };
  static const bool animated_red_blue_want_overwrites[] = {
      true,
      false,
      false,
      false,
  };

  struct {
    const char* filename;
    uint64_t want_count;
    uint32_t want_num_animation_loops;
    const wuffs_base__rect_ie_u32* want_bounds;
    const uint64_t* want_io_ps;
    const uint64_t* want_durations;
    const bool* want_overwrites;
  } test_cases[] = {
      {
          .filename =
              "test/data/hippopotamus.masked-with-muybridge.lossless.webp",
          .want_count = WUFFS_TESTLIB_ARRAY_SIZE(hippopotamus_want_bounds),
          .want_num_animation_loops = 0,
          .want_bounds = hippopotamus_want_bounds,
          .want_io_ps = hippopotamus_want_io_ps,
          .want_durations = hippopotamus_want_durations,
          .want_overwrites = hippopotamus_want_overwrites,
      },
      {
          .filename = "test/data/animated-red-blue.webp",
          .want_count = WUFFS_TESTLIB_ARRAY_SIZE(animated_red_blue_want_bounds),
          .want_num_animation_loops = 3,
          .want_bounds = animated_red_blue_want_bounds,
          .want_io_ps = animated_red_blue_want_io_ps,
          .want_durations = animated_red_blue_want_durations,
          .want_overwrites = animated_red_blue_want_overwrites,
      },
  };

  for (size_t tc = 0; tc < WUFFS_TESTLIB_ARRAY_SIZE(test_cases); tc++) {
    wuffs_webp__decoder* dec = &g_webp_decoder;
    CHECK_STATUS("initialize",
                 wuffs_webp__decoder__initialize(
                     dec, sizeof *dec, WUFFS_VERSION,
                     WUFFS_INITIALIZE__LEAVE_INTERNAL_BUFFERS_UNINITIALIZED));
    wuffs_base__frame_config fc = ((wuffs_base__frame_config){});
    wuffs_base__io_buffer src = ((wuffs_base__io_buffer){
        .data = g_src_slice_u8,
    });
    CHECK_STRING(read_file(&src, test_cases[tc].filename));

    uint64_t have_count = 0;
    while (true) {
      wuffs_base__status status =
          wuffs_webp__decoder__decode_frame_config(dec, &fc, &src);
      if (status.repr == wuffs_base__note__end_of_data) {
        break;
      } else if (!wuffs_base__status__is_ok(&status)) {
        RETURN_FAIL("decode_frame_config tc=%zu #%" PRIu64 ": %s", tc,
                    have_count, status.repr);
      }

      if (have_count < test_cases[tc].want_count) {
        wuffs_base__rect_ie_u32 have_bounds =
            wuffs_base__frame_config__bounds(&fc);
        const wuffs_base__rect_ie_u32* want_bounds =
            &test_cases[tc].want_bounds[have_count];
        if (!wuffs_base__rect_ie_u32__equals(&have_bounds, *want_bounds)) {
          RETURN_FAIL("bounds tc=%zu #%" PRIu64 ": have (%" PRIu32 ", %" PRIu32
                      ")-(%" PRIu32 ", %" PRIu32 "), want (%" PRIu32
                      ", %" PRIu32 ")-(%" PRIu32 ", %" PRIu32 ")",
                      tc, have_count, have_bounds.min_incl_x,
                      have_bounds.min_incl_y, have_bounds.max_excl_x,
                      have_bounds.max_excl_y, want_bounds->min_incl_x,
                      want_bounds->min_incl_y, want_bounds->max_excl_x,
                      want_bounds->max_excl_y);
        }

        uint64_t have_io_p = wuffs_base__frame_config__io_position(&fc);
        if (have_io_p != test_cases[tc].want_io_ps[have_count]) {
          RETURN_FAIL("io_position tc=%zu #%" PRIu64 ": have %" PRIu64
                      ", want %" PRIu64,
                      tc, have_count, have_io_p,
                      test_cases[tc].want_io_ps[have_count]);
        }

        uint64_t have_duration =
            (uint64_t)wuffs_base__frame_config__duration(&fc);
        if (have_duration != test_cases[tc].want_durations[have_count]) {
          RETURN_FAIL("duration tc=%zu #%" PRIu64 ": have %" PRIu64
                      ", want %" PRIu64,
                      tc, have_count, have_duration,
                      test_cases[tc].want_durations[have_count]);
        }

        bool have_overwrite =
            wuffs_base__frame_config__overwrite_instead_of_blend(&fc);
        if (have_overwrite != test_cases[tc].want_overwrites[have_count]) {
          RETURN_FAIL("overwrite_instead_of_blend tc=%zu #%" PRIu64
                      ": have %d, want %d",
                      tc, have_count, (int)have_overwrite,
                      (int)test_cases[tc].want_overwrites[have_count]);
        }

        if (wuffs_base__frame_config__disposal(&fc) !=
            WUFFS_BASE__ANIMATION_DISPOSAL__NONE) {
          RETURN_FAIL("disposal tc=%zu #%" PRIu64 ": have %d, want 0", tc,
                      have_count, (int)wuffs_base__frame_config__disposal(&fc));
        }

        // The ANIM chunk's background color (opaque black in
        // animated-red-blue.webp) is only a hint.
        if (wuffs_base__frame_config__background_color(&fc) != 0) {
          RETURN_FAIL(
              "background_color tc=%zu #%" PRIu64 ": have 0x%08" PRIX32
              ", want 0",
              tc, have_count, wuffs_base__frame_config__background_color(&fc));
        }

        uint64_t have_index = wuffs_base__frame_config__index(&fc);
        if (have_index != have_count) {
          RETURN_FAIL("index tc=%zu #%" PRIu64 ": have %" PRIu64, tc,
                      have_count, have_index);
        }
      }

      have_count++;
    }

    if (have_count != test_cases[tc].want_count) {
      RETURN_FAIL("count tc=%zu: have %" PRIu64 ", want %" PRIu64, tc,
                  have_count, test_cases[tc].want_count);
    }
    if (wuffs_webp__decoder__num_decoded_frame_configs(dec) != have_count) {
      RETURN_FAIL(
          "num_decoded_frame_configs tc=%zu: have %" PRIu64 ", want %" PRIu64,
          tc, wuffs_webp__decoder__num_decoded_frame_configs(dec), have_count);
    }
    // Skipping (not decoding) a frame still counts as decoding it.
    if (wuffs_webp__decoder__num_decoded_frames(dec) != have_count) {
      RETURN_FAIL("num_decoded_frames tc=%zu: have %" PRIu64 ", want %" PRIu64,
                  tc, wuffs_webp__decoder__num_decoded_frames(dec), have_count);
    }
    uint32_t have_loops = wuffs_webp__decoder__num_animation_loops(dec);
    if (have_loops != test_cases[tc].want_num_animation_loops) {
      RETURN_FAIL("num_animation_loops tc=%zu: have %" PRIu32 ", want %" PRIu32,
                  tc, have_loops, test_cases[tc].want_num_animation_loops);
    }
  }

  return NULL;
}

const char*  //
test_wuffs_webp_decode_inconsistent_width_and_height() {
  CHECK_FOCUS(__func__);
  wuffs_webp__decoder* dec = &g_webp_decoder;
  CHECK_STATUS("initialize",
               wuffs_webp__decoder__initialize(
                   dec, sizeof *dec, WUFFS_VERSION,
                   WUFFS_INITIALIZE__LEAVE_INTERNAL_BUFFERS_UNINITIALIZED));

  wuffs_base__io_buffer src = ((wuffs_base__io_buffer){
      .data = g_src_slice_u8,
  });
  CHECK_STRING(read_file(
      &src, "test/data/hippopotamus.masked-with-muybridge.lossless.webp"));

  // Change the VP8X chunk's canvas width (minus one) from 35 to 34, so that it
  // no longer matches the VP8L chunk's 36x28.
  if ((src.meta.wi < 27) || (src.data.ptr[24] != 35)) {
    RETURN_FAIL("unexpected VP8X chunk");
  }
  src.data.ptr[24] = 34;

  wuffs_base__image_config ic = ((wuffs_base__image_config){});
  CHECK_STATUS("decode_image_config",
               wuffs_webp__decoder__decode_image_config(dec, &ic, &src));
  const uint32_t w = 35;
  const uint32_t h = 28;
  if ((wuffs_base__pixel_config__width(&ic.pixcfg) != w) ||
      (wuffs_base__pixel_config__height(&ic.pixcfg) != h)) {
    RETURN_FAIL("dimensions: have %" PRIu32 "x%" PRIu32 ", want 35x28",
                wuffs_base__pixel_config__width(&ic.pixcfg),
                wuffs_base__pixel_config__height(&ic.pixcfg));
  }

  wuffs_base__pixel_buffer pb = ((wuffs_base__pixel_buffer){});
  CHECK_STATUS("set_from_slice", wuffs_base__pixel_buffer__set_from_slice(
                                     &pb, &ic.pixcfg, g_pixel_slice_u8));
  wuffs_base__status status = wuffs_webp__decoder__decode_frame(
      dec, &pb, &src, WUFFS_BASE__PIXEL_BLEND__SRC, g_work_slice_u8, NULL);
  if (status.repr != wuffs_webp__error__inconsistent_width_and_height) {
    RETURN_FAIL("decode_frame: have \"%s\", want \"%s\"", status.repr,
                wuffs_webp__error__inconsistent_width_and_height);
  }

  // The frame_dirty_rect should still be within the image.
  wuffs_base__rect_ie_u32 r = wuffs_webp__decoder__frame_dirty_rect(dec);
  if ((r.max_excl_x > w) || (r.max_excl_y > h)) {
    RETURN_FAIL("frame_dirty_rect: have (%" PRIu32 ", %" PRIu32 ")-(%" PRIu32
                ", %" PRIu32 "), want within (0, 0)-(35, 28)",
                r.min_incl_x, r.min_incl_y, r.max_excl_x, r.max_excl_y);
  }
  return NULL;
}

const char*  //
test_wuffs_webp_decode_interface_lossless() {
  CHECK_FOCUS(__func__);
  wuffs_webp__decoder* dec = &g_webp_decoder;
  CHECK_STATUS("initialize",
               wuffs_webp__decoder__initialize(
                   dec, sizeof *dec, WUFFS_VERSION,
                   WUFFS_INITIALIZE__LEAVE_INTERNAL_BUFFERS_UNINITIALIZED));
  return do_test__wuffs_base__image_decoder(
      wuffs_webp__decoder__upcast_as__wuffs_base__image_decoder(dec),
      "test/data/bricks-color.lossless.webp", 160, 120, 0xFF022460, NULL);
}

const char*  //
test_wuffs_webp_decode_interface_lossy() {
  CHECK_FOCUS(__func__);
  wuffs_webp__decoder* dec = &g_webp_decoder;
  CHECK_STATUS("initialize",
               wuffs_webp__decoder__initialize(
                   dec, sizeof *dec, WUFFS_VERSION,
                   WUFFS_INITIALIZE__LEAVE_INTERNAL_BUFFERS_UNINITIALIZED));
  return do_test__wuffs_base__image_decoder(
      wuffs_webp__decoder__upcast_as__wuffs_base__image_decoder(dec),
      "test/data/bricks-color.lossy.webp", 160, 120, 0xFF032665, NULL);
}

const char*  //
test_wuffs_webp_decode_interface_narrow_back_reference() {
  CHECK_FOCUS(__func__);
  wuffs_webp__decoder* dec = &g_webp_decoder;
  CHECK_STATUS("initialize",
               wuffs_webp__decoder__initialize(
                   dec, sizeof *dec, WUFFS_VERSION,
                   WUFFS_INITIALIZE__LEAVE_INTERNAL_BUFFERS_UNINITIALIZED));
  return do_test__wuffs_base__image_decoder(
      wuffs_webp__decoder__upcast_as__wuffs_base__image_decoder(dec),
      "test/data/artificial-webp/narrow-back-reference.webp", 1, 4, 0xFF4080C0,
      NULL);
}

const char*  //
test_wuffs_webp_decode_interface_vp8x_alpha_lossless() {
  CHECK_FOCUS(__func__);
  wuffs_webp__decoder* dec = &g_webp_decoder;
  CHECK_STATUS("initialize",
               wuffs_webp__decoder__initialize(
                   dec, sizeof *dec, WUFFS_VERSION,
                   WUFFS_INITIALIZE__LEAVE_INTERNAL_BUFFERS_UNINITIALIZED));
  return do_test__wuffs_base__image_decoder(
      wuffs_webp__decoder__upcast_as__wuffs_base__image_decoder(dec),
      "test/data/hippopotamus.masked-with-muybridge.lossless.webp", 36, 28,
      0x7D787878, NULL);
}

const char*  //
test_wuffs_webp_decode_interface_vp8x_alpha_lossy() {
  CHECK_FOCUS(__func__);
  wuffs_webp__decoder* dec = &g_webp_decoder;
  CHECK_STATUS("initialize",
               wuffs_webp__decoder__initialize(
                   dec, sizeof *dec, WUFFS_VERSION,
                   WUFFS_INITIALIZE__LEAVE_INTERNAL_BUFFERS_UNINITIALIZED));
  return do_test__wuffs_base__image_decoder(
      wuffs_webp__decoder__upcast_as__wuffs_base__image_decoder(dec),
      "test/data/hippopotamus.masked-with-muybridge.lossy.webp", 36, 28,
      0x7D787878, NULL);
}

const char*  //
test_wuffs_webp_decode_minimum_workbuf_len() {
  CHECK_FOCUS(__func__);

  struct {
    const char* filename;
    uint32_t want_width;
    uint32_t want_height;
    wuffs_base__color_u32_argb_premul want_final_pixel;
  } test_cases[] = {
      {
          .filename = "test/data/bricks-color.lossless.webp",
          .want_width = 160,
          .want_height = 120,
          .want_final_pixel = 0xFF022460,
      },
      {
          .filename = "test/data/bricks-color.lossy.webp",
          .want_width = 160,
          .want_height = 120,
          .want_final_pixel = 0xFF032665,
      },
      {
          .filename =
              "test/data/hippopotamus.masked-with-muybridge.36x8.lossy.webp",
          .want_width = 36,
          .want_height = 8,
          .want_final_pixel = 0xFFC8EFE2,
      },
      {
          .filename =
              "test/data/hippopotamus.masked-with-muybridge.lossless.webp",
          .want_width = 36,
          .want_height = 28,
          .want_final_pixel = 0x7D787878,
      },
      {
          .filename = "test/data/hippopotamus.masked-with-muybridge.lossy.webp",
          .want_width = 36,
          .want_height = 28,
          .want_final_pixel = 0x7D787878,
      },
  };

  for (size_t tc = 0; tc < WUFFS_TESTLIB_ARRAY_SIZE(test_cases); tc++) {
    wuffs_webp__decoder* dec = &g_webp_decoder;
    CHECK_STATUS("initialize",
                 wuffs_webp__decoder__initialize(
                     dec, sizeof *dec, WUFFS_VERSION,
                     WUFFS_INITIALIZE__LEAVE_INTERNAL_BUFFERS_UNINITIALIZED));
    do_test__wuffs_base__image_decoder__options options = {0};
    options.use_the_minimum_workbuf_len = true;
    CHECK_STRING(do_test__wuffs_base__image_decoder(
        wuffs_webp__decoder__upcast_as__wuffs_base__image_decoder(dec),
        test_cases[tc].filename, test_cases[tc].want_width,
        test_cases[tc].want_height, test_cases[tc].want_final_pixel, &options));
  }
  return NULL;
}

const char*  //
do_test_wuffs_webp_decode_many_small_reads(
    const char* filename,
    uint32_t want_width,
    uint32_t want_height,
    uint32_t want_pixfmt_repr,
    int want_num_iters_greater_than,
    wuffs_base__color_u32_argb_premul want_last_pixel) {
  wuffs_webp__decoder* dec = &g_webp_decoder;
  CHECK_STATUS("initialize",
               wuffs_webp__decoder__initialize(
                   dec, sizeof *dec, WUFFS_VERSION,
                   WUFFS_INITIALIZE__LEAVE_INTERNAL_BUFFERS_UNINITIALIZED));

  wuffs_base__io_buffer src = ((wuffs_base__io_buffer){
      .data = g_src_slice_u8,
  });
  CHECK_STRING(read_file(&src, filename));

  wuffs_base__image_config ic = ((wuffs_base__image_config){});
  CHECK_STATUS("decode_image_config",
               wuffs_webp__decoder__decode_image_config(dec, &ic, &src));

  const uint32_t w = want_width;
  if (wuffs_base__pixel_config__width(&ic.pixcfg) != w) {
    RETURN_FAIL("width: have %" PRIu32 ", want %" PRIu32,
                wuffs_base__pixel_config__width(&ic.pixcfg), w);
  }
  const uint32_t h = want_height;
  if (wuffs_base__pixel_config__height(&ic.pixcfg) != h) {
    RETURN_FAIL("height: have %" PRIu32 ", want %" PRIu32,
                wuffs_base__pixel_config__height(&ic.pixcfg), h);
  }
  if (wuffs_base__pixel_config__pixel_format(&ic.pixcfg).repr !=
      want_pixfmt_repr) {
    RETURN_FAIL("pixel_format: have 0x%08" PRIX32 ", want 0x%08" PRIX32,
                wuffs_base__pixel_config__pixel_format(&ic.pixcfg).repr,
                want_pixfmt_repr);
  }

  // 30 is 12 for the RIFF container header, 8 for the RIFF chunk header and
  // 10 for the VP8 (or VP8X) header.
  if (wuffs_base__image_config__first_frame_io_position(&ic) != 30) {
    RETURN_FAIL("first_frame_io_position: have %" PRIu64 ", want 30",
                wuffs_base__image_config__first_frame_io_position(&ic));
  }

  wuffs_base__pixel_buffer pb = ((wuffs_base__pixel_buffer){});
  CHECK_STATUS("set_from_slice", wuffs_base__pixel_buffer__set_from_slice(
                                     &pb, &ic.pixcfg, g_pixel_slice_u8));
  wuffs_base__pixel_buffer__set_color_u32_argb_premul_fill_rect(
      &pb, wuffs_base__make_rect_ie_u32(0, 0, w, h), 0xFF234567);

  wuffs_base__range_ii_u64 r = wuffs_webp__decoder__workbuf_len(dec);
  if (wuffs_base__range_ii_u64__is_empty(&r)) {
    RETURN_FAIL("workbuf_len: indeterminate workbuf length");
  }
  uint64_t m = r.min_incl;
  wuffs_base__slice_u8 workbuf = g_work_slice_u8;
  if (workbuf.len > m) {
    workbuf.len = m;
  }

  const uint64_t rlimit = 10;
  int num_iters = 0;
  while (true) {
    num_iters++;
    wuffs_base__io_buffer limited_src = make_limited_reader(src, rlimit);
    size_t old_ri = src.meta.ri;

    wuffs_base__status status = wuffs_webp__decoder__decode_frame(
        dec, &pb, &limited_src, WUFFS_BASE__PIXEL_BLEND__SRC, workbuf, NULL);
    src.meta.ri += limited_src.meta.ri;

    if (wuffs_base__status__is_ok(&status)) {
      break;
    }
    if (status.repr != wuffs_base__suspension__short_read) {
      RETURN_FAIL("decode_frame: have \"%s\", want \"%s\"", status.repr,
                  wuffs_base__suspension__short_read);
    }

    if (src.meta.ri < old_ri) {
      RETURN_FAIL("read index src.meta.ri went backwards");
    } else if (src.meta.ri == old_ri) {
      RETURN_FAIL("no progress was made");
    }
  }

  if (num_iters <= want_num_iters_greater_than) {
    RETURN_FAIL("num_iters: have %d, want > %d", num_iters,
                want_num_iters_greater_than);
  }

  wuffs_base__color_u32_argb_premul last_pixel =
      wuffs_base__pixel_buffer__color_u32_at(&pb, w - 1, h - 1);
  if (last_pixel != want_last_pixel) {
    RETURN_FAIL("last_pixel: have 0x%08" PRIX32 ", want 0x%08" PRIX32,
                last_pixel, want_last_pixel);
  }

  return NULL;
}

const char*  //
test_wuffs_webp_decode_many_small_reads() {
  CHECK_FOCUS(__func__);
  return do_test_wuffs_webp_decode_many_small_reads(
      "test/data/mona-lisa.21x32.q0.lossy.webp", 21, 32,
      WUFFS_BASE__PIXEL_FORMAT__BGRX, 1, 0xFF210A1B);
}

const char*  //
test_wuffs_webp_decode_many_small_reads_uncompressed_alph() {
  CHECK_FOCUS(__func__);
  // The ALPH chunk's payload is 1009 bytes: a 1 byte header and then 36 * 28
  // uncompressed alpha values. With 10 byte reads, that's over 100 reads.
  return do_test_wuffs_webp_decode_many_small_reads(
      "test/data/"
      "hippopotamus.masked-with-muybridge.uncompressed-alpha.lossy.webp",
      36, 28, WUFFS_BASE__PIXEL_FORMAT__BGRA_NONPREMUL, 100, 0x7D787878);
}

const char*  //
test_wuffs_webp_decode_restart_frame() {
  CHECK_FOCUS(__func__);

  struct {
    const char* filename;
    uint64_t want_ffio;
    wuffs_base__color_u32_argb_premul want_final_pixel;
  } test_cases[] = {
      {
          // RIFF, WEBP, VP8L and the 5 byte VP8L header.
          .filename = "test/data/bricks-color.lossless.webp",
          .want_ffio = 25,
          .want_final_pixel = 0xFF022460,
      },
      {
          // RIFF, WEBP, VP8 and the 10 byte VP8 header.
          .filename = "test/data/bricks-color.lossy.webp",
          .want_ffio = 30,
          .want_final_pixel = 0xFF032665,
      },
      {
          // RIFF, WEBP and VP8X.
          .filename =
              "test/data/hippopotamus.masked-with-muybridge.lossless.webp",
          .want_ffio = 30,
          .want_final_pixel = 0x7D787878,
      },
      {
          // RIFF, WEBP and VP8X.
          .filename = "test/data/hippopotamus.masked-with-muybridge.lossy.webp",
          .want_ffio = 30,
          .want_final_pixel = 0x7D787878,
      },
  };

  for (size_t tc = 0; tc < WUFFS_TESTLIB_ARRAY_SIZE(test_cases); tc++) {
    wuffs_base__io_buffer src = ((wuffs_base__io_buffer){
        .data = g_src_slice_u8,
    });
    CHECK_STRING(read_file(&src, test_cases[tc].filename));

    wuffs_webp__decoder* dec = &g_webp_decoder;
    CHECK_STATUS("initialize",
                 wuffs_webp__decoder__initialize(
                     dec, sizeof *dec, WUFFS_VERSION,
                     WUFFS_INITIALIZE__LEAVE_INTERNAL_BUFFERS_UNINITIALIZED));

    wuffs_base__image_config ic = ((wuffs_base__image_config){});
    CHECK_STATUS("decode_image_config",
                 wuffs_webp__decoder__decode_image_config(dec, &ic, &src));
    uint64_t ffio = wuffs_base__image_config__first_frame_io_position(&ic);
    if (ffio != test_cases[tc].want_ffio) {
      RETURN_FAIL("tc=%zu: first_frame_io_position: have %" PRIu64
                  ", want %" PRIu64,
                  tc, ffio, test_cases[tc].want_ffio);
    }
    wuffs_base__pixel_config__set(&ic.pixcfg,
                                  WUFFS_BASE__PIXEL_FORMAT__BGRA_PREMUL,
                                  WUFFS_BASE__PIXEL_SUBSAMPLING__NONE,
                                  wuffs_base__pixel_config__width(&ic.pixcfg),
                                  wuffs_base__pixel_config__height(&ic.pixcfg));

    wuffs_base__pixel_buffer pb = ((wuffs_base__pixel_buffer){});
    CHECK_STATUS("set_from_slice", wuffs_base__pixel_buffer__set_from_slice(
                                       &pb, &ic.pixcfg, g_pixel_slice_u8));

    wuffs_base__range_ii_u64 r = wuffs_webp__decoder__workbuf_len(dec);
    if (wuffs_base__range_ii_u64__is_empty(&r) ||
        (r.max_incl > g_work_slice_u8.len)) {
      RETURN_FAIL("tc=%zu: workbuf_len: bad range", tc);
    }
    wuffs_base__slice_u8 workbuf =
        wuffs_base__make_slice_u8(g_work_slice_u8.ptr, r.max_incl);

    for (int i = 0; i < 3; i++) {
      if (i > 0) {
        CHECK_STATUS("restart_frame",
                     wuffs_webp__decoder__restart_frame(dec, 0, ffio));
        src.meta.ri = ffio;

        if ((wuffs_webp__decoder__num_decoded_frame_configs(dec) != 0) ||
            (wuffs_webp__decoder__num_decoded_frames(dec) != 0)) {
          RETURN_FAIL(
              "tc=%zu: num_decoded_etc after restart #%d: have %" PRIu64
              ", %" PRIu64 ", want 0, 0",
              tc, i, wuffs_webp__decoder__num_decoded_frame_configs(dec),
              wuffs_webp__decoder__num_decoded_frames(dec));
        }
      }

      wuffs_base__frame_config fc = ((wuffs_base__frame_config){});
      wuffs_base__status status =
          wuffs_webp__decoder__decode_frame_config(dec, &fc, &src);
      if (!wuffs_base__status__is_ok(&status)) {
        RETURN_FAIL("tc=%zu: decode_frame_config #%d: %s", tc, i, status.repr);
      } else if (wuffs_base__frame_config__io_position(&fc) != ffio) {
        RETURN_FAIL("tc=%zu: io_position #%d: have %" PRIu64 ", want %" PRIu64,
                    tc, i, wuffs_base__frame_config__io_position(&fc), ffio);
      }

      status = wuffs_webp__decoder__decode_frame(
          dec, &pb, &src, WUFFS_BASE__PIXEL_BLEND__SRC, workbuf, NULL);
      if (!wuffs_base__status__is_ok(&status)) {
        RETURN_FAIL("tc=%zu: decode_frame #%d: %s", tc, i, status.repr);
      }

      wuffs_base__color_u32_argb_premul have_final_pixel =
          wuffs_base__pixel_buffer__color_u32_at(
              &pb, wuffs_base__pixel_config__width(&ic.pixcfg) - 1,
              wuffs_base__pixel_config__height(&ic.pixcfg) - 1);
      if (have_final_pixel != test_cases[tc].want_final_pixel) {
        RETURN_FAIL("tc=%zu: final pixel #%d: have 0x%08" PRIX32
                    ", want 0x%08" PRIX32,
                    tc, i, have_final_pixel, test_cases[tc].want_final_pixel);
      }

      if ((wuffs_webp__decoder__num_decoded_frame_configs(dec) != 1) ||
          (wuffs_webp__decoder__num_decoded_frames(dec) != 1)) {
        RETURN_FAIL("tc=%zu: num_decoded_etc #%d: have %" PRIu64 ", %" PRIu64
                    ", want 1, 1",
                    tc, i, wuffs_webp__decoder__num_decoded_frame_configs(dec),
                    wuffs_webp__decoder__num_decoded_frames(dec));
      }

      status = wuffs_webp__decoder__decode_frame_config(dec, &fc, &src);
      if (status.repr != wuffs_base__note__end_of_data) {
        RETURN_FAIL("tc=%zu: decode_frame_config #%d: have \"%s\", want \"%s\"",
                    tc, i, status.repr, wuffs_base__note__end_of_data);
      }
    }
  }

  return NULL;
}

const char*  //
test_wuffs_webp_decode_restart_frame_animated() {
  CHECK_FOCUS(__func__);

  const char* filenames[] = {
      "test/data/animated-red-blue.webp",
      "test/data/artificial-webp/animated-lossy-offsets.webp",
  };

  for (size_t tc = 0; tc < WUFFS_TESTLIB_ARRAY_SIZE(filenames); tc++) {
    wuffs_base__io_buffer src = ((wuffs_base__io_buffer){
        .data = g_src_slice_u8,
    });
    CHECK_STRING(read_file(&src, filenames[tc]));

    wuffs_webp__decoder* dec = &g_webp_decoder;
    CHECK_STATUS("initialize",
                 wuffs_webp__decoder__initialize(
                     dec, sizeof *dec, WUFFS_VERSION,
                     WUFFS_INITIALIZE__LEAVE_INTERNAL_BUFFERS_UNINITIALIZED));

    wuffs_base__image_config ic = ((wuffs_base__image_config){});
    CHECK_STATUS("decode_image_config",
                 wuffs_webp__decoder__decode_image_config(dec, &ic, &src));
    wuffs_base__pixel_config__set(&ic.pixcfg,
                                  WUFFS_BASE__PIXEL_FORMAT__BGRA_NONPREMUL,
                                  WUFFS_BASE__PIXEL_SUBSAMPLING__NONE,
                                  wuffs_base__pixel_config__width(&ic.pixcfg),
                                  wuffs_base__pixel_config__height(&ic.pixcfg));
    uint64_t n = wuffs_base__pixel_config__pixbuf_len(&ic.pixcfg);
    if (n > g_pixel_slice_u8.len) {
      RETURN_FAIL("tc=%zu: pixel buffer is too small", tc);
    }

    wuffs_base__pixel_buffer pb = ((wuffs_base__pixel_buffer){});
    CHECK_STATUS("set_from_slice", wuffs_base__pixel_buffer__set_from_slice(
                                       &pb, &ic.pixcfg, g_pixel_slice_u8));

    wuffs_base__range_ii_u64 r = wuffs_webp__decoder__workbuf_len(dec);
    if (wuffs_base__range_ii_u64__is_empty(&r) ||
        (r.max_incl > g_work_slice_u8.len)) {
      RETURN_FAIL("tc=%zu: workbuf_len: bad range", tc);
    }
    wuffs_base__slice_u8 workbuf =
        wuffs_base__make_slice_u8(g_work_slice_u8.ptr, r.max_incl);

    // Decode the frames (each to an all-zeroes pixel buffer) in the usual
    // order, then (after restart_frame calls) in reverse order.
    uint64_t io_positions[4] = {0};
    uint64_t hashes[4] = {0};
    for (int pass = 0; pass < 2; pass++) {
      for (int j = 0; j < 4; j++) {
        uint64_t i = pass ? (3 - j) : j;
        if (pass) {
          CHECK_STATUS("restart_frame", wuffs_webp__decoder__restart_frame(
                                            dec, i, io_positions[i]));
          src.meta.ri = io_positions[i];
        }

        wuffs_base__frame_config fc = ((wuffs_base__frame_config){});
        CHECK_STATUS("decode_frame_config",
                     wuffs_webp__decoder__decode_frame_config(dec, &fc, &src));
        if (pass == 0) {
          io_positions[i] = wuffs_base__frame_config__io_position(&fc);
        } else if (io_positions[i] !=
                   wuffs_base__frame_config__io_position(&fc)) {
          RETURN_FAIL("tc=%zu, i=%" PRIu64 ": io_position: have %" PRIu64
                      ", want %" PRIu64,
                      tc, i, wuffs_base__frame_config__io_position(&fc),
                      io_positions[i]);
        }
        if (wuffs_base__frame_config__index(&fc) != i) {
          RETURN_FAIL("tc=%zu, i=%" PRIu64 ": index: have %" PRIu64, tc, i,
                      wuffs_base__frame_config__index(&fc));
        }

        memset(g_pixel_slice_u8.ptr, 0, n);
        CHECK_STATUS(
            "decode_frame",
            wuffs_webp__decoder__decode_frame(
                dec, &pb, &src, WUFFS_BASE__PIXEL_BLEND__SRC, workbuf, NULL));
        if (wuffs_webp__decoder__num_decoded_frames(dec) != (i + 1)) {
          RETURN_FAIL("tc=%zu, i=%" PRIu64
                      ": num_decoded_frames: have %" PRIu64,
                      tc, i, wuffs_webp__decoder__num_decoded_frames(dec));
        }

        // FNV-1a.
        uint64_t hash = 0xCBF29CE484222325ul;
        for (uint64_t k = 0; k < n; k++) {
          hash ^= g_pixel_slice_u8.ptr[k];
          hash *= 0x100000001B3ul;
        }
        if (pass == 0) {
          hashes[i] = hash;
        } else if (hashes[i] != hash) {
          RETURN_FAIL("tc=%zu, i=%" PRIu64 ": pixels differ", tc, i);
        }
      }
    }
  }

  return NULL;
}

const char*  //
test_wuffs_webp_decode_riff_ends_in_chunk_header() {
  CHECK_FOCUS(__func__);

  // animated-red-blue.webp's first ANMF chunk header starts at offset 44, so
  // a RIFF chunk length of 38 (or 40 or 42) makes the RIFF chunk end 2 (or 4
  // or 6) bytes into it. The file has more bytes after that, but they're
  // outside the RIFF chunk.
  for (uint32_t riff_chunk_length = 38; riff_chunk_length <= 42;
       riff_chunk_length += 2) {
    for (int incremental = 0; incremental < 2; incremental++) {
      wuffs_base__io_buffer src = ((wuffs_base__io_buffer){
          .data = g_src_slice_u8,
      });
      CHECK_STRING(read_file(&src, "test/data/animated-red-blue.webp"));
      wuffs_base__poke_u32le__no_bounds_check(src.data.ptr + 4,
                                              riff_chunk_length);
      const size_t full_wi = src.meta.wi;

      wuffs_webp__decoder* dec = &g_webp_decoder;
      CHECK_STATUS("initialize",
                   wuffs_webp__decoder__initialize(
                       dec, sizeof *dec, WUFFS_VERSION,
                       WUFFS_INITIALIZE__LEAVE_INTERNAL_BUFFERS_UNINITIALIZED));

      // If incremental, make the source bytes available one at a time.
      wuffs_base__status status = wuffs_base__make_status(NULL);
      for (size_t wi = 1;; wi++) {
        src.meta.wi = (incremental && (wi < full_wi)) ? wi : full_wi;
        src.meta.closed = src.meta.wi == full_wi;
        status = wuffs_webp__decoder__decode_image_config(dec, NULL, &src);
        if ((status.repr != wuffs_base__suspension__short_read) ||
            src.meta.closed) {
          break;
        }
      }
      if (status.repr != wuffs_webp__error__short_chunk) {
        RETURN_FAIL("riff_chunk_length=%" PRIu32
                    ", incremental=%d: have \"%s\", want \"%s\"",
                    riff_chunk_length, incremental, status.repr,
                    wuffs_webp__error__short_chunk);
      }
    }
  }
  return NULL;
}

const char*  //
test_wuffs_webp_decode_animated_offsets() {
  CHECK_FOCUS(__func__);
  wuffs_webp__decoder* dec = &g_webp_decoder;
  CHECK_STATUS("initialize",
               wuffs_webp__decoder__initialize(
                   dec, sizeof *dec, WUFFS_VERSION,
                   WUFFS_INITIALIZE__LEAVE_INTERNAL_BUFFERS_UNINITIALIZED));

  wuffs_base__io_buffer src = ((wuffs_base__io_buffer){
      .data = g_src_slice_u8,
  });
  CHECK_STRING(
      read_file(&src, "test/data/artificial-webp/animated-lossy-offsets.webp"));

  wuffs_base__image_config ic = ((wuffs_base__image_config){});
  CHECK_STATUS("decode_image_config",
               wuffs_webp__decoder__decode_image_config(dec, &ic, &src));
  const uint32_t w = 38;
  const uint32_t h = 21;
  if ((wuffs_base__pixel_config__width(&ic.pixcfg) != w) ||
      (wuffs_base__pixel_config__height(&ic.pixcfg) != h)) {
    RETURN_FAIL("dimensions: have %" PRIu32 "x%" PRIu32 ", want 38x21",
                wuffs_base__pixel_config__width(&ic.pixcfg),
                wuffs_base__pixel_config__height(&ic.pixcfg));
  }
  wuffs_base__pixel_config__set(&ic.pixcfg,
                                WUFFS_BASE__PIXEL_FORMAT__BGRA_NONPREMUL,
                                WUFFS_BASE__PIXEL_SUBSAMPLING__NONE, w, h);

  wuffs_base__range_ii_u64 r = wuffs_webp__decoder__workbuf_len(dec);
  if (wuffs_base__range_ii_u64__is_empty(&r) ||
      (r.max_incl > g_work_slice_u8.len)) {
    RETURN_FAIL("workbuf_len: bad range");
  }
  wuffs_base__slice_u8 workbuf =
      wuffs_base__make_slice_u8(g_work_slice_u8.ptr, r.max_incl);

  // Decode each frame to its own pixel buffer, initially all zeroes.
  const size_t n = 4 * w * h;
  if ((4 * n) > g_pixel_slice_u8.len) {
    RETURN_FAIL("pixel buffer is too small");
  }
  memset(g_pixel_slice_u8.ptr, 0, 4 * n);

  static const uint32_t want_x0s[4] = {0, 2, 0, 14};
  static const uint32_t want_y0s[4] = {0, 12, 0, 2};
  static const uint32_t want_widths[4] = {36, 36, 24, 24};
  static const uint32_t want_heights[4] = {8, 8, 19, 19};

  for (int i = 0; i < 4; i++) {
    wuffs_base__frame_config fc = ((wuffs_base__frame_config){});
    CHECK_STATUS("decode_frame_config",
                 wuffs_webp__decoder__decode_frame_config(dec, &fc, &src));
    wuffs_base__rect_ie_u32 b = wuffs_base__frame_config__bounds(&fc);
    if ((b.min_incl_x != want_x0s[i]) || (b.min_incl_y != want_y0s[i]) ||
        (wuffs_base__rect_ie_u32__width(&b) != want_widths[i]) ||
        (wuffs_base__rect_ie_u32__height(&b) != want_heights[i])) {
      RETURN_FAIL("bounds #%d: have (%" PRIu32 ", %" PRIu32 ")-(%" PRIu32
                  ", %" PRIu32 ")",
                  i, b.min_incl_x, b.min_incl_y, b.max_excl_x, b.max_excl_y);
    }

    wuffs_base__pixel_buffer pb = ((wuffs_base__pixel_buffer){});
    CHECK_STATUS("set_from_slice", wuffs_base__pixel_buffer__set_from_slice(
                                       &pb, &ic.pixcfg,
                                       wuffs_base__make_slice_u8(
                                           g_pixel_slice_u8.ptr + (i * n), n)));
    CHECK_STATUS(
        "decode_frame",
        wuffs_webp__decoder__decode_frame(
            dec, &pb, &src, WUFFS_BASE__PIXEL_BLEND__SRC, workbuf, NULL));

    wuffs_base__rect_ie_u32 dirty = wuffs_webp__decoder__frame_dirty_rect(dec);
    if (!wuffs_base__rect_ie_u32__equals(&dirty, b)) {
      RETURN_FAIL("frame_dirty_rect #%d: have (%" PRIu32 ", %" PRIu32
                  ")-(%" PRIu32 ", %" PRIu32 ")",
                  i, dirty.min_incl_x, dirty.min_incl_y, dirty.max_excl_x,
                  dirty.max_excl_y);
    }
  }

  wuffs_base__status status =
      wuffs_webp__decoder__decode_frame_config(dec, NULL, &src);
  if (status.repr != wuffs_base__note__end_of_data) {
    RETURN_FAIL("decode_frame_config: have \"%s\", want \"%s\"", status.repr,
                wuffs_base__note__end_of_data);
  }

  // Frames 1 and 3 should match frames 0 and 2, offset. Elsewhere (outside of
  // each frame's bounds), the pixels should be untouched: still zero.
  for (int i = 0; i < 4; i++) {
    const uint8_t* have = g_pixel_slice_u8.ptr + (i * n);
    const uint8_t* want = g_pixel_slice_u8.ptr + ((i & 2) * n);
    for (uint32_t y = 0; y < h; y++) {
      for (uint32_t x = 0; x < w; x++) {
        uint32_t have_pixel =
            wuffs_base__peek_u32le__no_bounds_check(have + (4 * ((y * w) + x)));
        uint32_t want_pixel = 0;
        if ((want_x0s[i] <= x) && (x < (want_x0s[i] + want_widths[i])) &&
            (want_y0s[i] <= y) && (y < (want_y0s[i] + want_heights[i]))) {
          want_pixel = wuffs_base__peek_u32le__no_bounds_check(
              want + (4 * (((y - want_y0s[i]) * w) + (x - want_x0s[i]))));
        }
        if (have_pixel != want_pixel) {
          RETURN_FAIL("frame #%d, x=%" PRIu32 ", y=%" PRIu32
                      ": have 0x%08" PRIX32 ", want 0x%08" PRIX32,
                      i, x, y, have_pixel, want_pixel);
        }
      }
    }
  }

  return NULL;
}

const char*  //
test_wuffs_webp_decode_filter_alph() {
  CHECK_FOCUS(__func__);

  const int width = 41;
  const int height = 6;

  // src is arbitrary bytes, a subset of a noise texture.
  const uint8_t src[1][246] = {
      {
          0x3C, 0xD4, 0xB6, 0x77, 0x64, 0xAB, 0xEF, 0x03, 0x7D, 0x2F, 0xC0,
          0x24, 0x79, 0x3E, 0x87, 0x9A, 0x36, 0x7F, 0xBC, 0xF0, 0x42, 0xAF,
          0x2D, 0x63, 0x22, 0xC3, 0x4E, 0x33, 0xE9, 0x67, 0x9C, 0x01, 0xA9,
          0x23, 0x50, 0xDD, 0x36, 0x26, 0x4B, 0x94, 0xDC,

          0x14, 0xFB, 0x47, 0x5F, 0xF2, 0x13, 0x53, 0xE7, 0xD7, 0x28, 0x8A,
          0x07, 0xC6, 0x54, 0xE5, 0xB5, 0x0E, 0x78, 0xAE, 0x14, 0x84, 0xC0,
          0x20, 0x7D, 0x95, 0xEA, 0x06, 0x8A, 0xFA, 0xC3, 0x70, 0x5C, 0xA6,
          0x88, 0xCC, 0x09, 0xBA, 0xA6, 0x68, 0x1E, 0xB3,

          0x5A, 0xA5, 0x68, 0xFD, 0x18, 0x90, 0x70, 0x3A, 0xF0, 0x5F, 0xCA,
          0x44, 0x2B, 0x58, 0x12, 0xBD, 0x43, 0x61, 0xB0, 0x9D, 0x0E, 0x3F,
          0xEC, 0xB6, 0x36, 0x6E, 0x94, 0xDE, 0x2F, 0xCE, 0x77, 0x92, 0x11,
          0xE2, 0x37, 0x81, 0xA0, 0x46, 0xCD, 0xDF, 0xA3,

          0x05, 0x97, 0xDC, 0x73, 0xE4, 0x35, 0x6D, 0xF2, 0xC7, 0x1D, 0x56,
          0x7B, 0xCE, 0x2A, 0x17, 0x54, 0xE9, 0x21, 0x7E, 0x4E, 0xEB, 0x3B,
          0xF9, 0x47, 0xCA, 0x75, 0xB9, 0xD9, 0x0B, 0x5A, 0x27, 0x8A, 0x4D,
          0xF5, 0x1F, 0xB7, 0x8E, 0xD0, 0xA4, 0x28, 0x81,

          0xD9, 0x32, 0xE4, 0xA3, 0x8F, 0x65, 0xD8, 0x9B, 0x42, 0xB2, 0x04,
          0x8B, 0x19, 0xBE, 0x9B, 0x00, 0x51, 0x22, 0x31, 0xF6, 0x7C, 0xBF,
          0x1A, 0x6C, 0xB0, 0x3C, 0x63, 0x0A, 0xF9, 0x53, 0x15, 0x93, 0x67,
          0xB8, 0x4B, 0x01, 0xF4, 0x82, 0xBE, 0x11, 0xFD,

          0x61, 0xC4, 0xA4, 0x57, 0x6D, 0x2C, 0xDD, 0xED, 0xB1, 0x92, 0x63,
          0xA8, 0xE2, 0x35, 0xC8, 0x7F, 0xD5, 0x9D, 0x49, 0x85, 0xAE, 0xE2,
          0x41, 0x0C, 0xFE, 0x76, 0xC5, 0x20, 0x45, 0xAC, 0x30, 0xCB, 0x73,
          0x24, 0xEF, 0xDA, 0x80, 0xAB, 0x5E, 0x89, 0x6E,
      },
  };

  const uint8_t wants[3][246] = {
      {
          0x3C, 0x10, 0xC6, 0x3D, 0xA1, 0x4C, 0x3B, 0x3E, 0xBB, 0xEA, 0xAA,
          0xCE, 0x47, 0x85, 0x0C, 0xA6, 0xDC, 0x5B, 0x17, 0x07, 0x49, 0xF8,
          0x25, 0x88, 0xAA, 0x6D, 0xBB, 0xEE, 0xD7, 0x3E, 0xDA, 0xDB, 0x84,
          0xA7, 0xF7, 0xD4, 0x0A, 0x30, 0x7B, 0x0F, 0xEB,

          0x50, 0x4B, 0x92, 0xF1, 0xE3, 0xF6, 0x49, 0x30, 0x07, 0x2F, 0xB9,
          0xC0, 0x86, 0xDA, 0xBF, 0x74, 0x82, 0xFA, 0xA8, 0xBC, 0x40, 0x00,
          0x20, 0x9D, 0x32, 0x1C, 0x22, 0xAC, 0xA6, 0x69, 0xD9, 0x35, 0xDB,
          0x63, 0x2F, 0x38, 0xF2, 0x98, 0x00, 0x1E, 0xD1,

          0xAA, 0x4F, 0xB7, 0xB4, 0xCC, 0x5C, 0xCC, 0x06, 0xF6, 0x55, 0x1F,
          0x63, 0x8E, 0xE6, 0xF8, 0xB5, 0xF8, 0x59, 0x09, 0xA6, 0xB4, 0xF3,
          0xDF, 0x95, 0xCB, 0x39, 0xCD, 0xAB, 0xDA, 0xA8, 0x1F, 0xB1, 0xC2,
          0xA4, 0xDB, 0x5C, 0xFC, 0x42, 0x0F, 0xEE, 0x91,

          0xAF, 0x46, 0x22, 0x95, 0x79, 0xAE, 0x1B, 0x0D, 0xD4, 0xF1, 0x47,
          0xC2, 0x90, 0xBA, 0xD1, 0x25, 0x0E, 0x2F, 0xAD, 0xFB, 0xE6, 0x21,
          0x1A, 0x61, 0x2B, 0xA0, 0x59, 0x32, 0x3D, 0x97, 0xBE, 0x48, 0x95,
          0x8A, 0xA9, 0x60, 0xEE, 0xBE, 0x62, 0x8A, 0x0B,

          0x88, 0xBA, 0x9E, 0x41, 0xD0, 0x35, 0x0D, 0xA8, 0xEA, 0x9C, 0xA0,
          0x2B, 0x44, 0x02, 0x9D, 0x9D, 0xEE, 0x10, 0x41, 0x37, 0xB3, 0x72,
          0x8C, 0xF8, 0xA8, 0xE4, 0x47, 0x51, 0x4A, 0x9D, 0xB2, 0x45, 0xAC,
          0x64, 0xAF, 0xB0, 0xA4, 0x26, 0xE4, 0xF5, 0xF2,

          0xE9, 0xAD, 0x51, 0xA8, 0x15, 0x41, 0x1E, 0x0B, 0xBC, 0x4E, 0xB1,
          0x59, 0x3B, 0x70, 0x38, 0xB7, 0x8C, 0x29, 0x72, 0xF7, 0xA5, 0x87,
          0xC8, 0xD4, 0xD2, 0x48, 0x0D, 0x2D, 0x72, 0x1E, 0x4E, 0x19, 0x8C,
          0xB0, 0x9F, 0x79, 0xF9, 0xA4, 0x02, 0x8B, 0xF9,
      },

      {
          0x3C, 0x10, 0xC6, 0x3D, 0xA1, 0x4C, 0x3B, 0x3E, 0xBB, 0xEA, 0xAA,
          0xCE, 0x47, 0x85, 0x0C, 0xA6, 0xDC, 0x5B, 0x17, 0x07, 0x49, 0xF8,
          0x25, 0x88, 0xAA, 0x6D, 0xBB, 0xEE, 0xD7, 0x3E, 0xDA, 0xDB, 0x84,
          0xA7, 0xF7, 0xD4, 0x0A, 0x30, 0x7B, 0x0F, 0xEB,

          0x50, 0x0B, 0x0D, 0x9C, 0x93, 0x5F, 0x8E, 0x25, 0x92, 0x12, 0x34,
          0xD5, 0x0D, 0xD9, 0xF1, 0x5B, 0xEA, 0xD3, 0xC5, 0x1B, 0xCD, 0xB8,
          0x45, 0x05, 0x3F, 0x57, 0xC1, 0x78, 0xD1, 0x01, 0x4A, 0x37, 0x2A,
          0x2F, 0xC3, 0xDD, 0xC4, 0xD6, 0xE3, 0x2D, 0x9E,

          0xAA, 0xB0, 0x75, 0x99, 0xAB, 0xEF, 0xFE, 0x5F, 0x82, 0x71, 0xFE,
          0x19, 0x38, 0x31, 0x03, 0x18, 0x2D, 0x34, 0x75, 0xB8, 0xDB, 0xF7,
          0x31, 0xBB, 0x75, 0xC5, 0x55, 0x56, 0x00, 0xCF, 0xC1, 0xC9, 0x3B,
          0x11, 0xFA, 0x5E, 0x64, 0x1C, 0xB0, 0x0C, 0x41,

          0xAF, 0x47, 0x51, 0x0C, 0x8F, 0x24, 0x6B, 0x51, 0x49, 0x8E, 0x54,
          0x94, 0x06, 0x5B, 0x1A, 0x6C, 0x16, 0x55, 0xF3, 0x06, 0xC6, 0x32,
          0x2A, 0x02, 0x3F, 0x3A, 0x0E, 0x2F, 0x0B, 0x29, 0xE8, 0x53, 0x88,
          0x06, 0x19, 0x15, 0xF2, 0xEC, 0x54, 0x34, 0xC2,

          0x88, 0x79, 0x35, 0xAF, 0x1E, 0x89, 0x43, 0xEC, 0x8B, 0x40, 0x58,
          0x1F, 0x1F, 0x19, 0xB5, 0x6C, 0x67, 0x77, 0x24, 0xFC, 0x42, 0xF1,
          0x44, 0x6E, 0xEF, 0x76, 0x71, 0x39, 0x04, 0x7C, 0xFD, 0xE6, 0xEF,
          0xBE, 0x64, 0x16, 0xE6, 0x6E, 0x12, 0x45, 0xBF,

          0xE9, 0x3D, 0xD9, 0x06, 0x8B, 0xB5, 0x20, 0xD9, 0x3C, 0xD2, 0xBB,
          0xC7, 0x01, 0x4E, 0x7D, 0xEB, 0x3C, 0x14, 0x6D, 0x81, 0xF0, 0xD3,
          0x85, 0x7A, 0xED, 0xEC, 0x36, 0x59, 0x49, 0x28, 0x2D, 0xB1, 0x62,
          0xE2, 0x53, 0xF0, 0x66, 0x19, 0x70, 0xCE, 0x2D,
      },

      {
          0x3C, 0x10, 0xC6, 0x3D, 0xA1, 0x4C, 0x3B, 0x3E, 0xBB, 0xEA, 0xAA,
          0xCE, 0x47, 0x85, 0x0C, 0xA6, 0xDC, 0x5B, 0x17, 0x07, 0x49, 0xF8,
          0x25, 0x88, 0xAA, 0x6D, 0xBB, 0xEE, 0xD7, 0x3E, 0xDA, 0xDB, 0x84,
          0xA7, 0xF7, 0xD4, 0x0A, 0x30, 0x7B, 0x0F, 0xEB,

          0x50, 0x1F, 0x1C, 0x5F, 0xB5, 0x73, 0xB5, 0x9F, 0xD6, 0x27, 0x8A,
          0xB5, 0xF4, 0x53, 0xE5, 0xB4, 0xF8, 0xEF, 0x59, 0x5D, 0x23, 0x92,
          0x20, 0x00, 0xB7, 0x64, 0xB8, 0x75, 0x58, 0xC3, 0x6F, 0xCC, 0x1B,
          0xC6, 0xCB, 0xB1, 0xBA, 0x86, 0x39, 0x1E, 0xAD,

          0xAA, 0x1E, 0x83, 0xC3, 0x17, 0x90, 0x42, 0x66, 0x8D, 0x5F, 0x8C,
          0xFB, 0x2A, 0x58, 0xFC, 0x88, 0x0F, 0x67, 0xB0, 0x51, 0x25, 0xD3,
          0x4D, 0xE3, 0x35, 0x6E, 0x56, 0xF1, 0x03, 0x3C, 0x77, 0x66, 0x11,
          0x9E, 0xDA, 0x41, 0xEA, 0xFC, 0x7C, 0x40, 0x72,

          0xAF, 0xBA, 0xDB, 0x72, 0xE4, 0x34, 0x6D, 0x83, 0x71, 0x60, 0xE3,
          0x7A, 0xCE, 0x26, 0xE1, 0xC1, 0x31, 0xAA, 0x71, 0x60, 0x1F, 0x08,
          0xF9, 0x46, 0xCA, 0x74, 0x15, 0x89, 0x0B, 0x9E, 0x00, 0x8A, 0x82,
          0xF4, 0x1E, 0xB7, 0x8D, 0x6F, 0xA4, 0x90, 0x43,

          0x88, 0xC5, 0xCA, 0x04, 0x05, 0x65, 0x76, 0x27, 0x57, 0xF8, 0x03,
          0x8B, 0xF8, 0x0E, 0x64, 0x44, 0x51, 0xEC, 0xE4, 0xC9, 0x04, 0xBF,
          0x19, 0x6C, 0xA0, 0x86, 0x8A, 0x08, 0xF9, 0x52, 0x15, 0x32, 0x91,
          0xB7, 0x4B, 0xE5, 0xAF, 0x13, 0x06, 0x11, 0xFD,

          0xE9, 0xC3, 0x6C, 0x57, 0xC5, 0x2B, 0x19, 0xED, 0xB0, 0x91, 0x63,
          0x93, 0xE1, 0x35, 0x53, 0xB2, 0x94, 0x9C, 0xDD, 0x47, 0xAE, 0xE1,
          0x7C, 0xDB, 0xFD, 0x59, 0x22, 0x20, 0x44, 0xAC, 0x9F, 0x87, 0x59,
          0xA3, 0x26, 0x9A, 0xE4, 0xF3, 0x44, 0xD8, 0x6D,
      },
  };

  wuffs_webp__decoder dec = {0};
  dec.private_impl.f_frame_width = width;
  dec.private_impl.f_frame_height = height;

  for (int filter = 0; filter < 3; filter++) {
    uint8_t buf[246];
    memcpy(&buf[0], &src[0][0], 246);
    wuffs_base__slice_u8 s = wuffs_base__make_slice_u8(&buf[0], 246);

    switch (filter) {
      case 0:
        wuffs_webp__decoder__filter_alph_horizontal(&dec, s);
        break;
      case 1:
        wuffs_webp__decoder__filter_alph_vertical(&dec, s);
        break;
      case 2:
        wuffs_webp__decoder__filter_alph_gradient(&dec, s);
        break;
    }

    if (0 == memcmp(&buf[0], &wants[filter][0], 246)) {
      continue;
    }

    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        int i = (y * width) + x;
        int have = buf[i];
        int want = wants[filter][i];
        if (have != want) {
          RETURN_FAIL("filter=%d, y=%d, x=%d: have 0x%02X, want 0x%02X", filter,
                      y, x, have, want);
        }
      }
    }

    return "unreachable";
  }
  return NULL;
}

// ---------------- Mimic Tests

#ifdef WUFFS_MIMIC

const char*  //
do_test_mimic_webp_decode(const char* filename) {
  wuffs_base__io_buffer src = ((wuffs_base__io_buffer){
      .data = g_src_slice_u8,
  });
  CHECK_STRING(read_file(&src, filename));

  src.meta.ri = 0;
  wuffs_base__io_buffer have = ((wuffs_base__io_buffer){
      .data = g_have_slice_u8,
  });
  CHECK_STRING(wuffs_webp_decode(
      NULL, &have, WUFFS_INITIALIZE__DEFAULT_OPTIONS,
      wuffs_base__make_pixel_format(WUFFS_BASE__PIXEL_FORMAT__BGRA_NONPREMUL),
      NULL, 0, &src));

  src.meta.ri = 0;
  wuffs_base__io_buffer want = ((wuffs_base__io_buffer){
      .data = g_want_slice_u8,
  });
  CHECK_STRING(mimic_webp_decode(
      NULL, &want, WUFFS_INITIALIZE__DEFAULT_OPTIONS,
      wuffs_base__make_pixel_format(WUFFS_BASE__PIXEL_FORMAT__BGRA_NONPREMUL),
      NULL, 0, &src));

  return check_io_buffers_equal("", &have, &want);
}

const char*  //
test_mimic_webp_lossless_decode_image_19k_8bpp() {
  CHECK_FOCUS(__func__);
  return do_test_mimic_webp_decode("test/data/bricks-gray.lossless.webp");
}

const char*  //
test_mimic_webp_lossless_decode_image_40k_24bpp() {
  CHECK_FOCUS(__func__);
  return do_test_mimic_webp_decode("test/data/hat.lossless.webp");
}

const char*  //
test_mimic_webp_lossless_decode_image_77k_8bpp() {
  CHECK_FOCUS(__func__);
  return do_test_mimic_webp_decode("test/data/bricks-dither.lossless.webp");
}

const char*  //
test_mimic_webp_lossless_decode_image_552k_32bpp() {
  CHECK_FOCUS(__func__);
  return do_test_mimic_webp_decode(
      "test/data/hibiscus.primitive.lossless.webp");
}

const char*  //
test_mimic_webp_lossless_decode_image_4002k_24bpp() {
  CHECK_FOCUS(__func__);
  return do_test_mimic_webp_decode("test/data/harvesters.lossless.webp");
}

const char*  //
test_mimic_webp_lossless_decode_narrow_back_reference() {
  CHECK_FOCUS(__func__);
  return do_test_mimic_webp_decode(
      "test/data/artificial-webp/narrow-back-reference.webp");
}

const char*  //
test_mimic_webp_lossless_decode_vp8x_alpha() {
  CHECK_FOCUS(__func__);
  return do_test_mimic_webp_decode(
      "test/data/hippopotamus.masked-with-muybridge.lossless.webp");
}

const char*  //
test_mimic_webp_lossy_decode_absolute_segment_filter_levels() {
  CHECK_FOCUS(__func__);
  return do_test_mimic_webp_decode(
      "test/data/artificial-webp/absolute-segment-filter-levels.webp");
}

const char*  //
test_mimic_webp_lossy_decode_filter_clip() {
  CHECK_FOCUS(__func__);
  return do_test_mimic_webp_decode("test/data/hippopotamus.q0.lossy.webp");
}

const char*  //
test_mimic_webp_lossy_decode_image_19k_8bpp() {
  CHECK_FOCUS(__func__);
  return do_test_mimic_webp_decode("test/data/bricks-gray.lossy.webp");
}

const char*  //
test_mimic_webp_lossy_decode_image_40k_24bpp() {
  CHECK_FOCUS(__func__);
  return do_test_mimic_webp_decode("test/data/hat.lossy.webp");
}

const char*  //
test_mimic_webp_lossy_decode_image_77k_24bpp() {
  CHECK_FOCUS(__func__);
  return do_test_mimic_webp_decode("test/data/bricks-color.lossy.webp");
}

const char*  //
test_mimic_webp_lossy_decode_image_552k_32bpp() {
  CHECK_FOCUS(__func__);
  return do_test_mimic_webp_decode("test/data/hibiscus.primitive.lossy.webp");
}

const char*  //
test_mimic_webp_lossy_decode_image_4002k_24bpp() {
  CHECK_FOCUS(__func__);
  return do_test_mimic_webp_decode("test/data/harvesters.lossy.webp");
}

const char*  //
test_mimic_webp_lossy_decode_sharpness() {
  CHECK_FOCUS(__func__);
  return do_test_mimic_webp_decode(
      "test/data/hippopotamus.sharpness3.lossy.webp");
}

const char*  //
test_mimic_webp_lossy_decode_uncompressed_alph() {
  CHECK_FOCUS(__func__);
  return do_test_mimic_webp_decode(
      "test/data/"
      "hippopotamus.masked-with-muybridge.uncompressed-alpha.lossy.webp");
}

const char*  //
test_mimic_webp_lossy_decode_vp8x_alpha() {
  CHECK_FOCUS(__func__);
  return do_test_mimic_webp_decode(
      "test/data/hippopotamus.masked-with-muybridge.36x8.lossy.webp");
}

#endif  // WUFFS_MIMIC

// ---------------- WebP Benches

const char*  //
bench_wuffs_webp_lossless_decode_image_19k_8bpp() {
  CHECK_FOCUS(__func__);
  return do_bench_image_decode(
      &wuffs_webp_decode,
      WUFFS_INITIALIZE__LEAVE_INTERNAL_BUFFERS_UNINITIALIZED,
      wuffs_base__make_pixel_format(WUFFS_BASE__PIXEL_FORMAT__Y), NULL, 0,
      "test/data/bricks-gray.lossless.webp", 0, SIZE_MAX, 50);
}

const char*  //
bench_wuffs_webp_lossless_decode_image_40k_24bpp() {
  CHECK_FOCUS(__func__);
  return do_bench_image_decode(
      &wuffs_webp_decode,
      WUFFS_INITIALIZE__LEAVE_INTERNAL_BUFFERS_UNINITIALIZED,
      wuffs_base__make_pixel_format(WUFFS_BASE__PIXEL_FORMAT__BGRA_NONPREMUL),
      NULL, 0, "test/data/hat.lossless.webp", 0, SIZE_MAX, 30);
}

const char*  //
bench_wuffs_webp_lossless_decode_image_77k_8bpp() {
  CHECK_FOCUS(__func__);
  return do_bench_image_decode(
      &wuffs_webp_decode,
      WUFFS_INITIALIZE__LEAVE_INTERNAL_BUFFERS_UNINITIALIZED,
      wuffs_base__make_pixel_format(WUFFS_BASE__PIXEL_FORMAT__BGRA_NONPREMUL),
      NULL, 0, "test/data/bricks-dither.lossless.webp", 0, SIZE_MAX, 50);
}

const char*  //
bench_wuffs_webp_lossless_decode_image_552k_32bpp() {
  uint32_t q = WUFFS_BASE__QUIRK_IGNORE_CHECKSUM;
  CHECK_FOCUS(__func__);
  return do_bench_image_decode(
      &wuffs_webp_decode,
      WUFFS_INITIALIZE__LEAVE_INTERNAL_BUFFERS_UNINITIALIZED,
      wuffs_base__make_pixel_format(WUFFS_BASE__PIXEL_FORMAT__BGRA_NONPREMUL),
      &q, 1, "test/data/hibiscus.primitive.lossless.webp", 0, SIZE_MAX, 4);
}

const char*  //
bench_wuffs_webp_lossless_decode_image_4002k_24bpp() {
  CHECK_FOCUS(__func__);
  return do_bench_image_decode(
      &wuffs_webp_decode,
      WUFFS_INITIALIZE__LEAVE_INTERNAL_BUFFERS_UNINITIALIZED,
      wuffs_base__make_pixel_format(WUFFS_BASE__PIXEL_FORMAT__BGRA_NONPREMUL),
      NULL, 0, "test/data/harvesters.lossless.webp", 0, SIZE_MAX, 1);
}

const char*  //
bench_wuffs_webp_lossy_decode_image_19k_8bpp() {
  CHECK_FOCUS(__func__);
  return do_bench_image_decode(
      &wuffs_webp_decode,
      WUFFS_INITIALIZE__LEAVE_INTERNAL_BUFFERS_UNINITIALIZED,
      wuffs_base__make_pixel_format(WUFFS_BASE__PIXEL_FORMAT__Y), NULL, 0,
      "test/data/bricks-gray.lossy.webp", 0, SIZE_MAX, 50);
}

const char*  //
bench_wuffs_webp_lossy_decode_image_40k_24bpp() {
  CHECK_FOCUS(__func__);
  return do_bench_image_decode(
      &wuffs_webp_decode,
      WUFFS_INITIALIZE__LEAVE_INTERNAL_BUFFERS_UNINITIALIZED,
      wuffs_base__make_pixel_format(WUFFS_BASE__PIXEL_FORMAT__BGRA_NONPREMUL),
      NULL, 0, "test/data/hat.lossy.webp", 0, SIZE_MAX, 30);
}

const char*  //
bench_wuffs_webp_lossy_decode_image_77k_24bpp() {
  CHECK_FOCUS(__func__);
  return do_bench_image_decode(
      &wuffs_webp_decode,
      WUFFS_INITIALIZE__LEAVE_INTERNAL_BUFFERS_UNINITIALIZED,
      wuffs_base__make_pixel_format(WUFFS_BASE__PIXEL_FORMAT__BGRA_NONPREMUL),
      NULL, 0, "test/data/bricks-color.lossy.webp", 0, SIZE_MAX, 50);
}

const char*  //
bench_wuffs_webp_lossy_decode_image_552k_32bpp() {
  uint32_t q = WUFFS_BASE__QUIRK_IGNORE_CHECKSUM;
  CHECK_FOCUS(__func__);
  return do_bench_image_decode(
      &wuffs_webp_decode,
      WUFFS_INITIALIZE__LEAVE_INTERNAL_BUFFERS_UNINITIALIZED,
      wuffs_base__make_pixel_format(WUFFS_BASE__PIXEL_FORMAT__BGRA_NONPREMUL),
      &q, 1, "test/data/hibiscus.primitive.lossy.webp", 0, SIZE_MAX, 4);
}

const char*  //
bench_wuffs_webp_lossy_decode_image_4002k_24bpp() {
  CHECK_FOCUS(__func__);
  return do_bench_image_decode(
      &wuffs_webp_decode,
      WUFFS_INITIALIZE__LEAVE_INTERNAL_BUFFERS_UNINITIALIZED,
      wuffs_base__make_pixel_format(WUFFS_BASE__PIXEL_FORMAT__BGRA_NONPREMUL),
      NULL, 0, "test/data/harvesters.lossy.webp", 0, SIZE_MAX, 1);
}

// ---------------- Mimic Benches

#ifdef WUFFS_MIMIC

const char*  //
bench_mimic_webp_lossless_decode_image_19k_8bpp() {
  CHECK_FOCUS(__func__);
  return do_bench_image_decode(
      &mimic_webp_decode,
      WUFFS_INITIALIZE__LEAVE_INTERNAL_BUFFERS_UNINITIALIZED,
      wuffs_base__make_pixel_format(WUFFS_BASE__PIXEL_FORMAT__Y), NULL, 0,
      "test/data/bricks-gray.lossless.webp", 0, SIZE_MAX, 50);
}

const char*  //
bench_mimic_webp_lossless_decode_image_40k_24bpp() {
  CHECK_FOCUS(__func__);
  return do_bench_image_decode(
      &mimic_webp_decode,
      WUFFS_INITIALIZE__LEAVE_INTERNAL_BUFFERS_UNINITIALIZED,
      wuffs_base__make_pixel_format(WUFFS_BASE__PIXEL_FORMAT__BGRA_NONPREMUL),
      NULL, 0, "test/data/hat.lossless.webp", 0, SIZE_MAX, 30);
}

const char*  //
bench_mimic_webp_lossless_decode_image_77k_8bpp() {
  CHECK_FOCUS(__func__);
  return do_bench_image_decode(
      &mimic_webp_decode,
      WUFFS_INITIALIZE__LEAVE_INTERNAL_BUFFERS_UNINITIALIZED,
      wuffs_base__make_pixel_format(WUFFS_BASE__PIXEL_FORMAT__BGRA_NONPREMUL),
      NULL, 0, "test/data/bricks-dither.lossless.webp", 0, SIZE_MAX, 50);
}

const char*  //
bench_mimic_webp_lossless_decode_image_552k_32bpp() {
  uint32_t q = WUFFS_BASE__QUIRK_IGNORE_CHECKSUM;
  CHECK_FOCUS(__func__);
  return do_bench_image_decode(
      &mimic_webp_decode,
      WUFFS_INITIALIZE__LEAVE_INTERNAL_BUFFERS_UNINITIALIZED,
      wuffs_base__make_pixel_format(WUFFS_BASE__PIXEL_FORMAT__BGRA_NONPREMUL),
      &q, 1, "test/data/hibiscus.primitive.lossless.webp", 0, SIZE_MAX, 4);
}

const char*  //
bench_mimic_webp_lossless_decode_image_4002k_24bpp() {
  CHECK_FOCUS(__func__);
  return do_bench_image_decode(
      &mimic_webp_decode,
      WUFFS_INITIALIZE__LEAVE_INTERNAL_BUFFERS_UNINITIALIZED,
      wuffs_base__make_pixel_format(WUFFS_BASE__PIXEL_FORMAT__BGRA_NONPREMUL),
      NULL, 0, "test/data/harvesters.lossless.webp", 0, SIZE_MAX, 1);
}

const char*  //
bench_mimic_webp_lossy_decode_image_19k_8bpp() {
  CHECK_FOCUS(__func__);
  return do_bench_image_decode(
      &mimic_webp_decode,
      WUFFS_INITIALIZE__LEAVE_INTERNAL_BUFFERS_UNINITIALIZED,
      wuffs_base__make_pixel_format(WUFFS_BASE__PIXEL_FORMAT__Y), NULL, 0,
      "test/data/bricks-gray.lossy.webp", 0, SIZE_MAX, 50);
}

const char*  //
bench_mimic_webp_lossy_decode_image_40k_24bpp() {
  CHECK_FOCUS(__func__);
  return do_bench_image_decode(
      &mimic_webp_decode,
      WUFFS_INITIALIZE__LEAVE_INTERNAL_BUFFERS_UNINITIALIZED,
      wuffs_base__make_pixel_format(WUFFS_BASE__PIXEL_FORMAT__BGRA_NONPREMUL),
      NULL, 0, "test/data/hat.lossy.webp", 0, SIZE_MAX, 30);
}

const char*  //
bench_mimic_webp_lossy_decode_image_77k_24bpp() {
  CHECK_FOCUS(__func__);
  return do_bench_image_decode(
      &mimic_webp_decode,
      WUFFS_INITIALIZE__LEAVE_INTERNAL_BUFFERS_UNINITIALIZED,
      wuffs_base__make_pixel_format(WUFFS_BASE__PIXEL_FORMAT__BGRA_NONPREMUL),
      NULL, 0, "test/data/bricks-color.lossy.webp", 0, SIZE_MAX, 50);
}

const char*  //
bench_mimic_webp_lossy_decode_image_552k_32bpp() {
  uint32_t q = WUFFS_BASE__QUIRK_IGNORE_CHECKSUM;
  CHECK_FOCUS(__func__);
  return do_bench_image_decode(
      &mimic_webp_decode,
      WUFFS_INITIALIZE__LEAVE_INTERNAL_BUFFERS_UNINITIALIZED,
      wuffs_base__make_pixel_format(WUFFS_BASE__PIXEL_FORMAT__BGRA_NONPREMUL),
      &q, 1, "test/data/hibiscus.primitive.lossy.webp", 0, SIZE_MAX, 4);
}

const char*  //
bench_mimic_webp_lossy_decode_image_4002k_24bpp() {
  CHECK_FOCUS(__func__);
  return do_bench_image_decode(
      &mimic_webp_decode,
      WUFFS_INITIALIZE__LEAVE_INTERNAL_BUFFERS_UNINITIALIZED,
      wuffs_base__make_pixel_format(WUFFS_BASE__PIXEL_FORMAT__BGRA_NONPREMUL),
      NULL, 0, "test/data/harvesters.lossy.webp", 0, SIZE_MAX, 1);
}

#endif  // WUFFS_MIMIC

// ---------------- Manifest

proc g_tests[] = {

    test_wuffs_webp_decode_animated_offsets,
    test_wuffs_webp_decode_filter_alph,
    test_wuffs_webp_decode_frame_config,
    test_wuffs_webp_decode_inconsistent_width_and_height,
    test_wuffs_webp_decode_interface_lossless,
    test_wuffs_webp_decode_interface_lossy,
    test_wuffs_webp_decode_interface_narrow_back_reference,
    test_wuffs_webp_decode_interface_vp8x_alpha_lossless,
    test_wuffs_webp_decode_interface_vp8x_alpha_lossy,
    test_wuffs_webp_decode_many_small_reads,
    test_wuffs_webp_decode_many_small_reads_uncompressed_alph,
    test_wuffs_webp_decode_minimum_workbuf_len,
    test_wuffs_webp_decode_restart_frame,
    test_wuffs_webp_decode_restart_frame_animated,
    test_wuffs_webp_decode_riff_ends_in_chunk_header,

#ifdef WUFFS_MIMIC

    test_mimic_webp_lossless_decode_image_19k_8bpp,
    test_mimic_webp_lossless_decode_image_40k_24bpp,
    test_mimic_webp_lossless_decode_image_77k_8bpp,
    test_mimic_webp_lossless_decode_image_552k_32bpp,
    test_mimic_webp_lossless_decode_image_4002k_24bpp,
    test_mimic_webp_lossless_decode_narrow_back_reference,
    test_mimic_webp_lossless_decode_vp8x_alpha,
    test_mimic_webp_lossy_decode_absolute_segment_filter_levels,
    test_mimic_webp_lossy_decode_filter_clip,
    test_mimic_webp_lossy_decode_image_19k_8bpp,
    test_mimic_webp_lossy_decode_image_40k_24bpp,
    test_mimic_webp_lossy_decode_image_77k_24bpp,
    test_mimic_webp_lossy_decode_image_552k_32bpp,
    test_mimic_webp_lossy_decode_image_4002k_24bpp,
    test_mimic_webp_lossy_decode_sharpness,
    test_mimic_webp_lossy_decode_uncompressed_alph,
    test_mimic_webp_lossy_decode_vp8x_alpha,

#endif  // WUFFS_MIMIC

    NULL,
};

proc g_benches[] = {

    bench_wuffs_webp_lossless_decode_image_19k_8bpp,
    bench_wuffs_webp_lossless_decode_image_40k_24bpp,
    bench_wuffs_webp_lossless_decode_image_77k_8bpp,
    bench_wuffs_webp_lossless_decode_image_552k_32bpp,
    bench_wuffs_webp_lossless_decode_image_4002k_24bpp,
    bench_wuffs_webp_lossy_decode_image_19k_8bpp,
    bench_wuffs_webp_lossy_decode_image_40k_24bpp,
    bench_wuffs_webp_lossy_decode_image_77k_24bpp,
    bench_wuffs_webp_lossy_decode_image_552k_32bpp,
    bench_wuffs_webp_lossy_decode_image_4002k_24bpp,

#ifdef WUFFS_MIMIC

    bench_mimic_webp_lossless_decode_image_19k_8bpp,
    bench_mimic_webp_lossless_decode_image_40k_24bpp,
    bench_mimic_webp_lossless_decode_image_77k_8bpp,
    bench_mimic_webp_lossless_decode_image_552k_32bpp,
    bench_mimic_webp_lossless_decode_image_4002k_24bpp,
    bench_mimic_webp_lossy_decode_image_19k_8bpp,
    bench_mimic_webp_lossy_decode_image_40k_24bpp,
    bench_mimic_webp_lossy_decode_image_77k_24bpp,
    bench_mimic_webp_lossy_decode_image_552k_32bpp,
    bench_mimic_webp_lossy_decode_image_4002k_24bpp,

#endif  // WUFFS_MIMIC

    NULL,
};

int  //
main(int argc, char** argv) {
  g_proc_package_name = "std/webp";
  return test_main(argc, argv, g_tests, g_benches);
}
