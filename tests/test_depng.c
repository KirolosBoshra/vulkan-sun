#include "utils/depng.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifndef ASSETS_DIR
#define ASSETS_DIR "assets"
#endif

int main(void) {
  // Test 1: decode the real logo asset
  u8 *file = NULL;
  usize file_size = 0;
  assert(depng_load_file(ASSETS_DIR "/logo.png", &file, &file_size) ==
         DEPNG_OK);
  assert(file != NULL && file_size > 0);

  u8 *px = NULL;
  u32 w = 0, h = 0;
  assert(depng_decode_rgba(file, file_size, &px, &w, &h) == DEPNG_OK);
  assert(px != NULL);
  assert(w == 256 && h == 256);

  // Test 2: structural sanity — logo has visible content AND transparency,
  // proving the alpha channel decoded instead of being forced opaque/zero
  bool seen_opaque = false, seen_transparent = false;
  for (u32 i = 0; i < w * h; i++) {
    if (px[i * 4 + 3] != 0)
      seen_opaque = true;
    else
      seen_transparent = true;
  }
  assert(seen_opaque && seen_transparent);
  free(px);

  // Test 3: is_png rejects garbage
  u8 garbage[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  assert(!is_png(garbage, sizeof(garbage)));
  assert(!is_png(garbage, 4));
  assert(is_png(file, file_size));

  // Test 4: decode rejects non-PNG
  u8 *bad_px = (u8 *)0x1;
  u32 bad_w = 0, bad_h = 0;
  assert(depng_decode_rgba(garbage, sizeof(garbage), &bad_px, &bad_w,
                           &bad_h) == DEPNG_ERR_NOT_PNG);

  // Test 5: truncated file (cut into IEND) is detected
  assert(depng_decode_rgba(file, file_size - 10, &bad_px, &bad_w, &bad_h) ==
         DEPNG_ERR_TRUNCATED);

  // Test 6: corrupted byte in IEND CRC is detected
  u8 *corrupt = malloc(file_size);
  assert(corrupt != NULL);
  memcpy(corrupt, file, file_size);
  corrupt[file_size - 2] ^= 0xFF;
  assert(depng_decode_rgba(corrupt, file_size, &bad_px, &bad_w, &bad_h) ==
         DEPNG_ERR_BAD_CRC);
  free(corrupt);

  // Test 7: missing file reports IO error
  u8 *missing = NULL;
  usize missing_size = 0;
  assert(depng_load_file(ASSETS_DIR "/does_not_exist.png", &missing,
                         &missing_size) == DEPNG_ERR_IO);

  // Test 8: every known error code has a real message
  for (int e = DEPNG_OK; e <= DEPNG_ERR_BAD_DATA; e++)
    assert(strcmp(depng_error_str((DEPNG_ERROR)e), "unknown error") != 0);

  free(file);
  printf("All depng tests passed!\n");
  return 0;
}
