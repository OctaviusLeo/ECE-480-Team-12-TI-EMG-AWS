#include <string.h>
#include "save.h"

/* ---- weak flash HAL (override in your board layer) ---- */
__attribute__((weak)) bool flash_read(uint32_t addr, void* buf, uint32_t len){
  (void)addr; (void)buf; (void)len; return false; /* no flash by default */
}
__attribute__((weak)) bool flash_write(uint32_t addr, const void* buf, uint32_t len){
  (void)addr; (void)buf; (void)len; return false;
}
#define SAVE_ADDR  (0x0007F000u) /* adjust to a reserved 4KB sector */

/* ---- CRC32 (small, standard polynomial) ---- */
static uint32_t crc32(const void* data, uint32_t len){
  const uint8_t* p = (const uint8_t*)data;
  uint32_t c = 0xFFFFFFFFu;
  for (uint32_t i=0;i<len;i++){
    c ^= p[i];
    for (int k=0;k<8;k++) c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
  }
  return ~c;
}

/* ---- API ---- */
void save_defaults(save_t *s){
  memset(s, 0, sizeof(*s));
  s->version = 1;
}

bool save_load(save_t *s){
  save_t tmp;
  if (!flash_read(SAVE_ADDR, &tmp, sizeof(tmp))) return false;
  uint32_t crc = tmp.crc32; tmp.crc32 = 0;
  if (crc != crc32(&tmp, sizeof(tmp))) return false;
  *s = tmp;
  return true;
}

bool save_write(const save_t *s){
  save_t tmp = *s;
  tmp.crc32 = 0;
  tmp.crc32 = crc32(&tmp, sizeof(tmp));
  return flash_write(SAVE_ADDR, &tmp, sizeof(tmp));
}
