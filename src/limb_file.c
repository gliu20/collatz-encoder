#include <stdbool.h>
#include "limb_file.h"

size_t read_file(limb_dlist_t* ll, FILE *file) {
  limb_t limb;
  size_t bytes_read = 0;
  size_t units_read;
  fpos_t pos;

  while (true) {
    fgetpos(file, &pos);
    units_read = fread(&limb, sizeof(limb), 1, file);
    bytes_read += units_read * sizeof(limb_t);
    if (units_read != 1) {
      if (feof(file)) {
        // Rewind back to just b4 eof error and attempt to read bytes
        fsetpos(file, &pos);

        // Read last bytes
        unsigned char mini_limb[sizeof(limb)] = {0};
        size_t mini_limb_len = 0;

        // Read byte by byte until we fail
        while (fread(&mini_limb[mini_limb_len++], 1, 1, file) == 1);

        // Last mini_limb is always the failed byte so it doesn't
        // count as part of the length of the mini_limb
        mini_limb_len--;

        printf("info: read %zu eof bytes\n", mini_limb_len);

        // Reconstruct limb from mini limb
        limb = 0;
        for (size_t i = 0; i < mini_limb_len; i++) {
          printf("info: got byte %02x\n", mini_limb[i]);
          limb <<= 8;
          limb |= mini_limb[mini_limb_len - 1 - i];
        }

        printf("info: reconstructed limb: %016llx\n", limb);

        bytes_read += mini_limb_len;
        resize_limb_list_to_length(ll, ll->length + 1);
        insert_at_tail(ll, limb);

        break;
      }
      if (ferror(file)) return __SIZE_MAX__;
    }

    resize_limb_list_to_length(ll, ll->length + 1);
    insert_at_tail(ll, limb);
  }
  return bytes_read;
}

size_t write_file(limb_dlist_t* ll, FILE *file) {
  size_t bytes_write = 0;
  size_t units_write;

  // We can't write nothing
  if (ll->length == 0) return __SIZE_MAX__;
  
  for (size_t i = 0; i < ll->length - 1; i++) {
    units_write = fwrite(&LL_INDEX(ll, i), sizeof(limb_t), 1, file);
    bytes_write += units_write * sizeof(limb_t);
    if (units_write != 1) {
      return __SIZE_MAX__;
    }
  }

  // Decompose into mini limbs
  limb_t limb = LL_TAIL(ll);

  printf("info: got limb %016llx\n", limb);

  unsigned char mini_limb[sizeof(limb_t)] = {0};
  size_t mini_limb_len = sizeof(limb_t);

  for (size_t i = 0; i < sizeof(limb_t); i++) {
    mini_limb[i] = limb & 0xff;
    limb >>= 8;
    printf("info: got byte %02x\n", mini_limb[i]);
  }

  // Chop off the most significant bytes equal to zero
  for (int i = sizeof(limb_t) - 1; i >= 0; i--) {
    if (mini_limb[i] != 0) break;
    mini_limb_len--;
  }

  printf("info: wrote %zu eof bytes\n", mini_limb_len);

  // Write the mini-limbs byte by byte
  for (size_t i = 0; i < mini_limb_len; i++) {
    if (fwrite(&mini_limb[i], 1, 1, file) != 1) break;
    bytes_write++;
  }

  return bytes_write;
}
