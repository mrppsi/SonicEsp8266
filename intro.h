#ifndef INTRO_H
#define INTRO_H

#include <Arduino.h>

// =====================================================================
// ANIMACIÓN: INTRO (124x64 px)
// =====================================================================
namespace SonicIntro {
  // Asegúrate de que tu archivo se llame exactamente "intro.txt" en la carpeta
  #include "intro_data.h"

  const int TOTAL_FRAMES = 60; 
  const unsigned char* const frames[] PROGMEM = {
    epd_bitmap_frame_00_delay_0, epd_bitmap_frame_01_delay_0, epd_bitmap_frame_02_delay_0,
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
    epd_bitmap_frame_57_delay_0, epd_bitmap_frame_58_delay_0, epd_bitmap_frame_59_delay_0
  };
}

#endif