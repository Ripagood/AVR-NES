#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "globals.h"
#include "apu.h"
#include "nes.h"
struct apu {
  Byte pulse1_control;
  Byte pulse1_rampControl;
  Byte pulse1_fineTune;
  Byte pulse1_courseTune;
  Byte pulse2_control;
  Byte pulse2_rampControl;
  Byte pulse2_fineTune;
  Byte pulse2_courseTune;
  Byte triangle_control1;
  Byte triangle_control2;
  Byte triangle_frequency1;
  Byte triangle_frequency2;
  Byte noise_control1;
  Byte noise_frequency1;
  Byte noise_frequency2;
  Byte delta_control;
  Byte delta_da;
  Byte delta_address;
  Byte delta_dataLength;
};
APU apu_init(void) {
  APU apu = (APU) malloc(sizeof(struct apu));
  assert(apu != NULL);
  apu->pulse1_control = 0;
  apu->pulse1_rampControl = 0;
  apu->pulse1_fineTune = 0;
  apu->pulse1_courseTune = 0;
  apu->pulse2_control = 0;
  apu->pulse2_rampControl = 0;
  apu->pulse2_fineTune = 0;
  apu->pulse2_courseTune = 0;
  apu->triangle_control1 = 0;
  apu->triangle_control2 = 0;
  apu->triangle_frequency1 = 0;
  apu->triangle_frequency2 = 0;
  apu->noise_control1 = 0;
  apu->noise_frequency1 = 0;
  apu->noise_frequency2 = 0;
  apu->delta_control = 0;
  apu->delta_da = 0;
  apu->delta_address = 0;
  apu->delta_dataLength = 0;
  return apu;
}
void apu_destroy(APU apu) {
  assert(apu != NULL);
  free(apu);
}
void apu_step(NES nes) {
  assert(nes != NULL);
  // not implemented
  // debug_printf("apu_step\n");
}
