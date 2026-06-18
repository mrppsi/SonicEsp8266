#ifndef SONIC_H
#define SONIC_H

#include <Arduino.h>

// =====================================================================
// 1. ANIMACIÓN: IDLE (128x64 px)
// =====================================================================
namespace SonicIdle {
  #include "idle_data.h"

  const int TOTAL_FRAMES = 58; 
  const unsigned char* const frames[] PROGMEM = {
    epd_bitmap_frame_00_delay_1s, epd_bitmap_frame_01_delay_0, epd_bitmap_frame_02_delay_0,
    epd_bitmap_frame_03_delay_0, epd_bitmap_frame_04_delay_0, epd_bitmap_frame_05_delay_0,
    epd_bitmap_frame_06_delay_0, epd_bitmap_frame_07_delay_0, epd_bitmap_frame_08_delay_0,
    epd_bitmap_frame_09_delay_0, epd_bitmap_frame_10_delay_0, epd_bitmap_frame_11_delay_0,
    epd_bitmap_frame_12_delay_0, epd_bitmap_frame_13_delay_0, epd_bitmap_frame_14_delay_0,
    epd_bitmap_frame_15_delay_0, epd_bitmap_frame_16_delay_0, epd_bitmap_frame_17_delay_0,
    epd_bitmap_frame_18_delay_0, epd_bitmap_frame_19_delay_0, epd_bitmap_frame_20_delay_0,
    epd_bitmap_frame_21_delay_0, epd_bitmap_frame_22_delay_0, epd_bitmap_frame_23_delay_0,
    epd_bitmap_frame_24_delay_0, epd_bitmap_frame_25_delay_0, epd_bitmap_frame_26_delay_0,
    epd_bitmap_frame_27_delay_0, epd_bitmap_frame_28_delay_0, epd_bitmap_frame_29_delay_0,
    epd_bitmap_frame_30_delay_0, epd_bitmap_frame_31_delay_0, epd_bitmap_frame_32_delay_0,
    epd_bitmap_frame_33_delay_0, epd_bitmap_frame_34_delay_0, epd_bitmap_frame_35_delay_0,
    epd_bitmap_frame_36_delay_0, epd_bitmap_frame_37_delay_0, epd_bitmap_frame_38_delay_0,
    epd_bitmap_frame_39_delay_0, epd_bitmap_frame_40_delay_0, epd_bitmap_frame_41_delay_0,
    epd_bitmap_frame_42_delay_0, epd_bitmap_frame_43_delay_0, epd_bitmap_frame_44_delay_0,
    epd_bitmap_frame_45_delay_0, epd_bitmap_frame_46_delay_0, epd_bitmap_frame_47_delay_0,
    epd_bitmap_frame_48_delay_0, epd_bitmap_frame_49_delay_0, epd_bitmap_frame_50_delay_0,
    epd_bitmap_frame_51_delay_0, epd_bitmap_frame_52_delay_0, epd_bitmap_frame_53_delay_0,
    epd_bitmap_frame_54_delay_0, epd_bitmap_frame_55_delay_0, epd_bitmap_frame_56_delay_0,
    epd_bitmap_frame_57_delay_0
  };
}

// =====================================================================
// 2. ANIMACIÓN: JUMP (16x16 px)
// =====================================================================
namespace SonicJump {
  #include "jump_data.h"

  const int TOTAL_FRAMES = 5; 
  const unsigned char* const frames[] PROGMEM = {
    epd_bitmap_frame_0_delay_0, epd_bitmap_frame_1_delay_0, epd_bitmap_frame_2_delay_0,
    epd_bitmap_frame_3_delay_0, epd_bitmap_frame_4_delay_0
  };
}

// =====================================================================
// 3. ANIMACIÓN: RUN1 - TROTE (16x24 px)
// =====================================================================
namespace SonicTrote {
  #include "run1_data.h"

  const int TOTAL_FRAMES = 6; 
  const unsigned char* const frames[] PROGMEM = {
    epd_bitmap_frame_0_delay_0, epd_bitmap_frame_1_delay_0, epd_bitmap_frame_2_delay_0,
    epd_bitmap_frame_3_delay_0, epd_bitmap_frame_4_delay_0, epd_bitmap_frame_5_delay_0
  };
}

// =====================================================================
// 4. ANIMACIÓN: RUN2 - CORRER (16x24 px)
// =====================================================================
namespace SonicRun {
  #include "run2_data.h"

  const int TOTAL_FRAMES = 4; 
  const unsigned char* const frames[] PROGMEM = {
    epd_bitmap_frame_0_delay_0, epd_bitmap_frame_1_delay_0, epd_bitmap_frame_2_delay_0,
    epd_bitmap_frame_3_delay_0
  };
}

#endif