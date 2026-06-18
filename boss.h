#ifndef BOSS_H
#define BOSS_H

#include <Arduino.h>

namespace SonicBoss {
  #include "boss_data.h"

  const int ANCHO = 32;
  const int ALTO  = 32;

  // Animación "idle" (flotando / esperando), se usa en bucle
  const int TOTAL_FRAMES_IDLE = 2;
  const unsigned char* const framesIdle[] PROGMEM = {
    epd_bitmap_boss_idle_0, epd_bitmap_boss_idle_1
  };

  // Frame único mostrado al recibir un golpe (breve destello de daño)
  const unsigned char* const frameHit = epd_bitmap_boss_hit_0;

  // Frame único mostrado al ser derrotado
  const unsigned char* const frameDefeated = epd_bitmap_boss_defeated_0;
}

#endif
