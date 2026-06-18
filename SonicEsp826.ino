#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- CONFIGURACIÓN DE CONTROLES (Pines ESP8266) ---
#define BTN_LEFT  D3  // Botón izquierdo  (también: subir en menús)
#define BTN_RIGHT D4  // Botón derecho    (también: bajar en menús)
#define BTN_JUMP  D5  // Botón de salto   (también: seleccionar / Press Start)

// --- INCLUSIÓN DE ANIMACIONES ---
#include "intro.h"
#include "sonic.h"
#include "boss.h"

// =====================================================================
// ESTADOS DEL MOTOR DEL JUEGO
// =====================================================================
enum EstadoJuego { INTRO_NORMAL, JUGANDO, PAUSADO, PELEA_JEFE, NIVEL_COMPLETO, GAME_OVER };
EstadoJuego estadoActual = INTRO_NORMAL;
EstadoJuego estadoAntesDePausa = JUGANDO; // a dónde volver al cerrar el menú

enum EstadoSonic { IDLE, TROTANDO, CORRIENDO, SALTANDO };
EstadoSonic estadoSonic = IDLE;

// --- ESTADOS DEL MENÚ DE PAUSA ---
enum OpcionMenu { CONTINUAR, SALIR };
OpcionMenu opcionSeleccionada = CONTINUAR;

// =====================================================================
// VARIABLES DE TIEMPOS Y ANIMACIÓN
// =====================================================================
unsigned long tiempoAnterior = 0;
const long INTERVALO_FRAME = 60; // 60ms para un buen balance de velocidad
int frameActual = 0;

// --- Control de "Press Start" parpadeante ---
unsigned long tiempoParpadeo = 0;
const long INTERVALO_PARPADEO = 500; // ms que tarda en alternar visible/invisible
bool mostrarPressStart = true;

// --- Anti-rebote para el combo de 3 botones (abrir menú) y navegación ---
unsigned long tiempoUltimaEntradaMenu = 0;
const long DEBOUNCE_MENU = 220; // ms entre movimientos de menú para que no "vuele" el cursor
bool combo3BotonesArmado = true; // evita reabrir el menú si se mantienen los botones presionados

// =====================================================================
// VARIABLES DE LA FÍSICA DE SONIC
// =====================================================================
int sonicX = 20;
int sonicY = 40;
int velocidadY = 0;
bool estaEnElSuelo = true;
const int GRAVEDAD = 2;
const int FUERZA_SALTO = -10;
const int PISO_Y = 40;

// --- Vidas ---
int vidas = 3;
unsigned long tiempoInvulnerable = 0;
const long DURACION_INVULNERABLE = 1000; // ms de parpadeo tras recibir daño
bool esInvulnerable = false;

// =====================================================================
// ESTRUCTURA DEL NIVEL (camino recto hacia el jefe)
// =====================================================================
// Por ahora el nivel es un tramo recto y simple: piso + palmeras de
// fondo + rings recolectables, hasta llegar a la arena del jefe.
// =====================================================================
const int LARGO_NIVEL = 900;            // distancia total hasta llegar al jefe
long distanciaRecorrida = 0;            // distancia acumulada del nivel

int scrollFondo = 0;
unsigned long tiempoMoviendo = 0;

// --- Rings recolectables (suman al contador) ---
const int NUM_ANILLOS = 8;
int anillosX[NUM_ANILLOS] = {100, 220, 340, 420, 540, 650, 760, 850};
bool anilloRecogido[NUM_ANILLOS] = {false, false, false, false, false, false, false, false};
int contadorRings = 0; // contador de rings recolectados en el intento actual (se reinicia con el nivel)

void dibujarPiso(int scroll);
void dibujarPalmera(int x);
void dibujarAnillos(long distancia);
void reiniciarNivelCompleto();
void reiniciarPosicionSonic();

// =====================================================================
// VARIABLES DEL JEFE
// =====================================================================
int bossX = 100;
int bossY = 16;          // el jefe "flota" en la parte alta de la arena
int bossDirY = 1;        // dirección del flotado (sube/baja)
int bossVida = 4;        // golpes necesarios para vencerlo (saltar sobre él 4 veces)
unsigned long tiempoUltimoGolpeJefe = 0;
const long INVULNERABLE_JEFE = 600; // ms de invulnerabilidad del jefe tras un golpe
bool bossEsInvulnerable = false;
bool bossDerrotado = false;
unsigned long tiempoBossFrame = 0;
int bossFrameActual = 0;

// --- Proyectiles simples del jefe ---
struct Proyectil {
  int x, y;
  bool activo;
};
const int NUM_PROYECTILES = 2;
Proyectil proyectiles[NUM_PROYECTILES] = {{0,0,false}, {0,0,false}};
unsigned long tiempoUltimoDisparo = 0;
const long INTERVALO_DISPARO = 1800; // ms entre disparos del jefe

void iniciarPeleaJefe();
void actualizarJefe();
void dibujarJefe();

void setup() {
  Serial.begin(115200);

  // Configuramos los botones con resistencias Pull-Up
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_JUMP, INPUT_PULLUP);

  // Iniciamos la pantalla
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;);
  }

  display.clearDisplay();
  display.display();
}

// =====================================================================
// LOOP PRINCIPAL
// =====================================================================
void loop() {
  unsigned long tiempoActual = millis();

  // ---------------------------------------------------------------
  // DETECCIÓN DEL COMBO DE 3 BOTONES -> ABRIR MENÚ DE PAUSA
  // (Se evalúa SIEMPRE, fuera del intervalo de frame, para que sea
  //  responsivo incluso si el juego está en plena animación)
  // ---------------------------------------------------------------
  bool izqPresionado = (digitalRead(BTN_LEFT) == LOW);
  bool derPresionado  = (digitalRead(BTN_RIGHT) == LOW);
  bool saltoPresionado = (digitalRead(BTN_JUMP) == LOW);
  bool los3Presionados = izqPresionado && derPresionado && saltoPresionado;

  if (los3Presionados && combo3BotonesArmado &&
      (estadoActual == JUGANDO || estadoActual == PELEA_JEFE)) {
    estadoAntesDePausa = estadoActual;
    estadoActual = PAUSADO;
    opcionSeleccionada = CONTINUAR;
    combo3BotonesArmado = false; // hasta que se suelten los botones no se vuelve a armar
    tiempoUltimaEntradaMenu = tiempoActual;
  }

  // Rearmamos el combo solo cuando el usuario suelta los 3 botones
  if (!los3Presionados) {
    combo3BotonesArmado = true;
  }

  // --- CONTROL DE FRAMES (todo lo demás respeta el intervalo) ---
  if (tiempoActual - tiempoAnterior >= INTERVALO_FRAME) {
    tiempoAnterior = tiempoActual;

    switch (estadoActual) {

      // =================================================================
      // FASE 1: REPRODUCCIÓN DE LA INTRO
      // =================================================================
      case INTRO_NORMAL: {

        // Botón para saltar la intro / iniciar el juego
        if (saltoPresionado) {
          estadoActual = JUGANDO;
          estadoSonic = IDLE;
          frameActual = 0;
          reiniciarNivelCompleto();
          display.clearDisplay();
          delay(200);
          break;
        }

        display.clearDisplay();

        const unsigned char* spriteIntro = (const unsigned char*)pgm_read_ptr(&(SonicIntro::frames[frameActual % SonicIntro::TOTAL_FRAMES]));
        display.drawBitmap(2, 0, spriteIntro, 124, 64, SSD1306_WHITE);

        // --- "PRESS START" parpadeante, asociado al botón de salto ---
        if (tiempoActual - tiempoParpadeo >= INTERVALO_PARPADEO) {
          tiempoParpadeo = tiempoActual;
          mostrarPressStart = !mostrarPressStart;
        }
        if (mostrarPressStart) {
          display.setTextSize(1);
          display.setTextColor(SSD1306_WHITE);
          // Centrado aproximado para "PRESS START" (12 caracteres x 6px ~ 72px)
          display.setCursor(28, 54);
          display.print("PRESS START");
        }

        display.display();

        frameActual++;
        if (frameActual >= SonicIntro::TOTAL_FRAMES) {
          frameActual = 0; // Bucle infinito de la intro
        }
        break;
      }

      // =================================================================
      // FASE 2A: MENÚ DE PAUSA
      // =================================================================
      case PAUSADO: {
        // Navegación con anti-rebote por tiempo (no por flanco, para
        // mantener el mismo estilo "digitalRead directo" del resto del código)
        if (tiempoActual - tiempoUltimaEntradaMenu >= DEBOUNCE_MENU) {

          if (izqPresionado && !derPresionado) {
            // Izquierda = subir en el menú
            opcionSeleccionada = CONTINUAR;
            tiempoUltimaEntradaMenu = tiempoActual;
          } else if (derPresionado && !izqPresionado) {
            // Derecha = bajar en el menú
            opcionSeleccionada = SALIR;
            tiempoUltimaEntradaMenu = tiempoActual;
          } else if (saltoPresionado && !izqPresionado && !derPresionado) {
            // Salto = seleccionar
            tiempoUltimaEntradaMenu = tiempoActual;

            if (opcionSeleccionada == CONTINUAR) {
              estadoActual = estadoAntesDePausa;
            } else { // SALIR
              estadoActual = INTRO_NORMAL;
              frameActual = 0;
              reiniciarNivelCompleto();
            }
            display.clearDisplay();
            display.display();
            delay(200); // pequeño debounce extra al confirmar
            break;
          }
        }

        // --- Dibujado del overlay de pausa ---
        display.clearDisplay();
        display.drawRect(14, 14, 100, 36, SSD1306_WHITE);
        display.fillRect(16, 16, 96, 32, SSD1306_BLACK);
        display.drawRect(16, 16, 96, 32, SSD1306_WHITE);

        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(38, 19);
        display.print("PAUSA");

        // Opción CONTINUAR
        display.setCursor(30, 31);
        if (opcionSeleccionada == CONTINUAR) display.print("> CONTINUAR");
        else display.print("  CONTINUAR");

        // Opción SALIR
        display.setCursor(30, 40);
        if (opcionSeleccionada == SALIR) display.print("> SALIR");
        else display.print("  SALIR");

        display.display();
        break;
      }

      // =================================================================
      // FASE 2B: EL JUEGO ACTIVO (recorrido del nivel)
      // =================================================================
      case JUGANDO: {
        display.clearDisplay();

        bool moviendose = false;

        // 1. Detectar botones Izquierda / Derecha
        if (derPresionado) {
          sonicX += 3;
          scrollFondo -= 4;
          distanciaRecorrida += 4;
          moviendose = true;
        } else if (izqPresionado) {
          sonicX -= 3;
          scrollFondo += 4;
          if (distanciaRecorrida > 0) distanciaRecorrida -= 4; // no retrocede antes del inicio del nivel
          moviendose = true;
        }

        // 2. Detectar salto
        if (saltoPresionado && estaEnElSuelo) {
          velocidadY = FUERZA_SALTO;
          estaEnElSuelo = false;
          estadoSonic = SALTANDO;
          frameActual = 0;
        }

        // 3. Físicas de caída
        if (!estaEnElSuelo) {
          sonicY += velocidadY;
          velocidadY += GRAVEDAD;

          if (sonicY >= PISO_Y) {
            sonicY = PISO_Y;
            velocidadY = 0;
            estaEnElSuelo = true;
            estadoSonic = moviendose ? TROTANDO : IDLE;
            frameActual = 0;
          }
        } else {
          // Lógica de aceleración (Trote a Correr) o reposo (Idle)
          if (moviendose) {
            if (tiempoMoviendo == 0) tiempoMoviendo = tiempoActual;
            if (tiempoActual - tiempoMoviendo > 1500) estadoSonic = CORRIENDO;
            else estadoSonic = TROTANDO;
          } else {
            estadoSonic = IDLE;
            tiempoMoviendo = 0;
          }
        }

        // Límites lógicos de la pantalla para que Sonic no se salga de los bordes
        if (sonicX < 0) sonicX = 0;
        if (sonicX > 112) sonicX = 112;

        // --- ¿Llegamos al final del tramo recto? -> arena del jefe ---
        if (distanciaRecorrida >= LARGO_NIVEL) {
          iniciarPeleaJefe();
          break;
        }

        // Renderizado del Fondo Procedural (piso + palmera, simple y recto)
        dibujarPiso(scrollFondo % 16);
        dibujarPalmera((scrollFondo + 60) % 180);
        dibujarAnillos(distanciaRecorrida);

        // Renderizado del sprite de Sonic
        const unsigned char* spriteSonic = NULL;
        int maxFrames = 1;

        switch (estadoSonic) {
          case IDLE:
            spriteSonic = (const unsigned char*)pgm_read_ptr(&(SonicIdle::frames[frameActual % SonicIdle::TOTAL_FRAMES]));
            maxFrames = SonicIdle::TOTAL_FRAMES;
            display.drawBitmap(sonicX, sonicY, spriteSonic, 16, 24, SSD1306_WHITE);
            break;

          case TROTANDO:
            spriteSonic = (const unsigned char*)pgm_read_ptr(&(SonicTrote::frames[frameActual % SonicTrote::TOTAL_FRAMES]));
            maxFrames = SonicTrote::TOTAL_FRAMES;
            display.drawBitmap(sonicX, sonicY, spriteSonic, 16, 24, SSD1306_WHITE);
            break;

          case CORRIENDO:
            spriteSonic = (const unsigned char*)pgm_read_ptr(&(SonicRun::frames[frameActual % SonicRun::TOTAL_FRAMES]));
            maxFrames = SonicRun::TOTAL_FRAMES;
            display.drawBitmap(sonicX, sonicY, spriteSonic, 16, 24, SSD1306_WHITE);
            break;

          case SALTANDO:
            spriteSonic = (const unsigned char*)pgm_read_ptr(&(SonicJump::frames[frameActual % SonicJump::TOTAL_FRAMES]));
            maxFrames = SonicJump::TOTAL_FRAMES;
            display.drawBitmap(sonicX, sonicY + 8, spriteSonic, 16, 16, SSD1306_WHITE);
            break;
        }

        // --- HUD: vidas y contador de rings ---
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(2, 2);
        display.print("V:");
        display.print(vidas);

        display.setCursor(70, 2);
        display.print("RINGS:");
        display.print(contadorRings);

        display.display();

        frameActual++;
        if (frameActual >= maxFrames) {
          frameActual = 0;
        }
        break;
      }

      // =================================================================
      // FASE 3: PELEA CONTRA EL JEFE
      // =================================================================
      case PELEA_JEFE: {
        display.clearDisplay();

        bool moviendose = false;

        // Movimiento limitado a la arena del jefe (sin scroll de fondo)
        if (derPresionado) {
          sonicX += 3;
          moviendose = true;
        } else if (izqPresionado) {
          sonicX -= 3;
          moviendose = true;
        }
        if (sonicX < 0) sonicX = 0;
        if (sonicX > 112) sonicX = 112;

        // Salto
        if (saltoPresionado && estaEnElSuelo) {
          velocidadY = FUERZA_SALTO;
          estaEnElSuelo = false;
          estadoSonic = SALTANDO;
          frameActual = 0;
        }

        // Físicas de caída (idénticas a la fase normal)
        if (!estaEnElSuelo) {
          sonicY += velocidadY;
          velocidadY += GRAVEDAD;

          if (sonicY >= PISO_Y) {
            sonicY = PISO_Y;
            velocidadY = 0;
            estaEnElSuelo = true;
            estadoSonic = moviendose ? TROTANDO : IDLE;
            frameActual = 0;
          }
        } else {
          estadoSonic = moviendose ? TROTANDO : IDLE;
        }

        // Suelo fijo de la arena (sin scroll, es una sala de jefe)
        display.drawFastHLine(0, 63, 128, SSD1306_WHITE);

        // Lógica y dibujado del jefe (movimiento, proyectiles, colisiones)
        actualizarJefe();
        dibujarJefe();

        // --- Manejo de invulnerabilidad de Sonic tras recibir daño ---
        if (esInvulnerable && tiempoActual - tiempoInvulnerable >= DURACION_INVULNERABLE) {
          esInvulnerable = false;
        }

        // Dibujado de Sonic (parpadea si es invulnerable)
        bool dibujarSonic = true;
        if (esInvulnerable && ((tiempoActual / 100) % 2 == 0)) {
          dibujarSonic = false; // efecto de parpadeo
        }

        if (dibujarSonic) {
          const unsigned char* spriteSonic = NULL;
          int maxFrames = 1;

          switch (estadoSonic) {
            case IDLE:
              spriteSonic = (const unsigned char*)pgm_read_ptr(&(SonicIdle::frames[frameActual % SonicIdle::TOTAL_FRAMES]));
              maxFrames = SonicIdle::TOTAL_FRAMES;
              display.drawBitmap(sonicX, sonicY, spriteSonic, 16, 24, SSD1306_WHITE);
              break;
            case TROTANDO:
              spriteSonic = (const unsigned char*)pgm_read_ptr(&(SonicTrote::frames[frameActual % SonicTrote::TOTAL_FRAMES]));
              maxFrames = SonicTrote::TOTAL_FRAMES;
              display.drawBitmap(sonicX, sonicY, spriteSonic, 16, 24, SSD1306_WHITE);
              break;
            case CORRIENDO:
              spriteSonic = (const unsigned char*)pgm_read_ptr(&(SonicRun::frames[frameActual % SonicRun::TOTAL_FRAMES]));
              maxFrames = SonicRun::TOTAL_FRAMES;
              display.drawBitmap(sonicX, sonicY, spriteSonic, 16, 24, SSD1306_WHITE);
              break;
            case SALTANDO:
              spriteSonic = (const unsigned char*)pgm_read_ptr(&(SonicJump::frames[frameActual % SonicJump::TOTAL_FRAMES]));
              maxFrames = SonicJump::TOTAL_FRAMES;
              display.drawBitmap(sonicX, sonicY + 8, spriteSonic, 16, 16, SSD1306_WHITE);
              break;
          }

          frameActual++;
          if (frameActual >= maxFrames) frameActual = 0;
        }

        // --- Colisión Sonic <-> Jefe (saltar sobre su cabeza para dañarlo) ---
        if (!bossDerrotado) {
          int sonicTop = sonicY;
          int sonicBottom = sonicY + 24;
          int sonicLeft = sonicX;
          int sonicRight = sonicX + 16;

          int bossTop = bossY;
          int bossBottom = bossY + SonicBoss::ALTO;
          int bossLeft = bossX;
          int bossRight = bossX + SonicBoss::ANCHO;

          bool hayColision = sonicRight > bossLeft && sonicLeft < bossRight &&
                              sonicBottom > bossTop && sonicTop < bossBottom;

          if (hayColision) {
            // Golpe válido: Sonic está SALTANDO y cayendo sobre la mitad superior del jefe
            bool golpeValido = (estadoSonic == SALTANDO) && (velocidadY > 0) &&
                                (sonicBottom < bossTop + (SonicBoss::ALTO / 2) + 6);

            if (golpeValido && !bossEsInvulnerable) {
              bossVida--;
              bossEsInvulnerable = true;
              tiempoUltimoGolpeJefe = tiempoActual;
              // Pequeño rebote hacia arriba al golpear al jefe (feedback táctil)
              velocidadY = FUERZA_SALTO / 2;

              if (bossVida <= 0) {
                bossDerrotado = true;
              }
            } else if (!golpeValido && !esInvulnerable) {
              // Contacto lateral con el jefe = Sonic recibe daño
              vidas--;
              esInvulnerable = true;
              tiempoInvulnerable = tiempoActual;
              // Empuje hacia atrás
              sonicX = (sonicX < bossX) ? max(0, sonicX - 10) : min(112, sonicX + 10);

              if (vidas <= 0) {
                estadoActual = GAME_OVER;
              }
            }
          }
        } else {
          // Jefe derrotado -> tras una breve pausa, marcar nivel completo
          static unsigned long tiempoDerrota = 0;
          if (tiempoDerrota == 0) tiempoDerrota = tiempoActual;
          if (tiempoActual - tiempoDerrota > 1500) {
            estadoActual = NIVEL_COMPLETO;
            tiempoDerrota = 0;
          }
        }

        // --- Colisión Proyectiles del jefe <-> Sonic ---
        if (!esInvulnerable) {
          for (int i = 0; i < NUM_PROYECTILES; i++) {
            if (proyectiles[i].activo) {
              bool golpeado = (sonicX + 16 > proyectiles[i].x) && (sonicX < proyectiles[i].x + 4) &&
                               (sonicY + 24 > proyectiles[i].y) && (sonicY < proyectiles[i].y + 4);
              if (golpeado) {
                vidas--;
                esInvulnerable = true;
                tiempoInvulnerable = tiempoActual;
                proyectiles[i].activo = false;
                if (vidas <= 0) {
                  estadoActual = GAME_OVER;
                }
              }
            }
          }
        }

        // --- HUD de la pelea: vidas + barra de vida del jefe + rings ---
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(2, 2);
        display.print("V:");
        display.print(vidas);

        display.setCursor(2, 12);
        display.print("R:");
        display.print(contadorRings);

        // Barra de vida del jefe (arriba a la derecha)
        display.drawRect(70, 2, 56, 6, SSD1306_WHITE);
        int anchoVidaJefe = map(bossVida, 0, 4, 0, 54);
        if (anchoVidaJefe > 0) {
          display.fillRect(71, 3, anchoVidaJefe, 4, SSD1306_WHITE);
        }

        display.display();
        break;
      }

      // =================================================================
      // FASE 4: NIVEL COMPLETADO
      // =================================================================
      case NIVEL_COMPLETO: {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(28, 26);
        display.print("NIVEL SUPERADO");
        display.setCursor(20, 40);
        display.print("Pulsa SALTO");

        if (saltoPresionado) {
          estadoActual = INTRO_NORMAL;
          frameActual = 0;
          reiniciarNivelCompleto();
          delay(200);
        }

        display.display();
        break;
      }

      // =================================================================
      // FASE 5: GAME OVER
      // =================================================================
      case GAME_OVER: {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(38, 26);
        display.print("GAME OVER");
        display.setCursor(20, 40);
        display.print("Pulsa SALTO");

        if (saltoPresionado) {
          estadoActual = INTRO_NORMAL;
          frameActual = 0;
          reiniciarNivelCompleto();
          delay(200);
        }

        display.display();
        break;
      }
    }
  }
}

// =====================================================================
// REINICIO DE NIVEL / SONIC (usado al salir desde el menú, perder o ganar)
// =====================================================================
void reiniciarPosicionSonic() {
  sonicX = 20;
  sonicY = PISO_Y;
  velocidadY = 0;
  estaEnElSuelo = true;
  estadoSonic = IDLE;
  esInvulnerable = false;
}

void reiniciarNivelCompleto() {
  reiniciarPosicionSonic();
  vidas = 3;
  distanciaRecorrida = 0;
  scrollFondo = 0;
  tiempoMoviendo = 0;
  contadorRings = 0;

  for (int i = 0; i < NUM_ANILLOS; i++) anilloRecogido[i] = false;

  bossVida = 4;
  bossDerrotado = false;
  bossEsInvulnerable = false;
  bossX = 100;
  bossY = 16;
  bossDirY = 1;
  for (int i = 0; i < NUM_PROYECTILES; i++) proyectiles[i].activo = false;
}

// =====================================================================
// INICIO DE LA PELEA CONTRA EL JEFE (transición desde el nivel)
// =====================================================================
void iniciarPeleaJefe() {
  estadoActual = PELEA_JEFE;
  reiniciarPosicionSonic();
  sonicX = 10; // Sonic entra por la izquierda de la arena del jefe
  bossX = 100;
  bossY = 16;
  bossDirY = 1;
  bossVida = 4;
  bossDerrotado = false;
  bossEsInvulnerable = false;
  for (int i = 0; i < NUM_PROYECTILES; i++) proyectiles[i].activo = false;
  display.clearDisplay();
  display.display();
}

// =====================================================================
// LÓGICA DEL JEFE: movimiento, disparo de proyectiles e invulnerabilidad
// =====================================================================
void actualizarJefe() {
  unsigned long tiempoActual = millis();

  if (!bossDerrotado) {
    // Movimiento flotante simple (sube y baja dentro de un rango)
    bossY += bossDirY;
    if (bossY <= 8) bossDirY = 1;
    if (bossY >= 24) bossDirY = -1;

    // Termina la invulnerabilidad del jefe tras el tiempo definido
    if (bossEsInvulnerable && (tiempoActual - tiempoUltimoGolpeJefe >= INVULNERABLE_JEFE)) {
      bossEsInvulnerable = false;
    }

    // Disparo periódico de proyectiles hacia donde está Sonic
    if (tiempoActual - tiempoUltimoDisparo >= INTERVALO_DISPARO) {
      tiempoUltimoDisparo = tiempoActual;
      for (int i = 0; i < NUM_PROYECTILES; i++) {
        if (!proyectiles[i].activo) {
          proyectiles[i].activo = true;
          proyectiles[i].x = bossX;
          proyectiles[i].y = bossY + (SonicBoss::ALTO / 2);
          break;
        }
      }
    }
  }

  // Actualizar movimiento de proyectiles activos (vuelan hacia la izquierda)
  for (int i = 0; i < NUM_PROYECTILES; i++) {
    if (proyectiles[i].activo) {
      proyectiles[i].x -= 4;
      if (proyectiles[i].x < -4) {
        proyectiles[i].activo = false;
      }
    }
  }
}

// =====================================================================
// DIBUJADO DEL JEFE Y SUS PROYECTILES
// =====================================================================
void dibujarJefe() {
  unsigned long tiempoActual = millis();

  if (bossDerrotado) {
    display.drawBitmap(bossX, bossY, SonicBoss::frameDefeated, SonicBoss::ANCHO, SonicBoss::ALTO, SSD1306_WHITE);
    return;
  }

  // Parpadeo del jefe mientras es invulnerable (feedback de golpe recibido)
  bool dibujarJefeAhora = true;
  if (bossEsInvulnerable && ((tiempoActual / 80) % 2 == 0)) {
    dibujarJefeAhora = false;
  }

  if (dibujarJefeAhora) {
    if (bossEsInvulnerable) {
      // Frame de "golpeado" justo al recibir el daño
      display.drawBitmap(bossX, bossY, SonicBoss::frameHit, SonicBoss::ANCHO, SonicBoss::ALTO, SSD1306_WHITE);
    } else {
      // Animación idle normal en bucle
      if (tiempoActual - tiempoBossFrame >= 250) {
        tiempoBossFrame = tiempoActual;
        bossFrameActual = (bossFrameActual + 1) % SonicBoss::TOTAL_FRAMES_IDLE;
      }
      const unsigned char* spriteBoss = (const unsigned char*)pgm_read_ptr(&(SonicBoss::framesIdle[bossFrameActual]));
      display.drawBitmap(bossX, bossY, spriteBoss, SonicBoss::ANCHO, SonicBoss::ALTO, SSD1306_WHITE);
    }
  }

  // Dibujar proyectiles activos (pequeños cuadrados de 4x4)
  for (int i = 0; i < NUM_PROYECTILES; i++) {
    if (proyectiles[i].activo) {
      display.fillRect(proyectiles[i].x, proyectiles[i].y, 4, 4, SSD1306_WHITE);
    }
  }
}

// =====================================================================
// FUNCIONES GRÁFICAS PROCEDURALES DEL ESCENARIO (TRAMO RECTO SIMPLE)
// =====================================================================

// Piso base con bloques tipo "ladrillo" desplazándose (igual que el original)
void dibujarPiso(int scroll) {
  display.drawFastHLine(0, 63, 128, SSD1306_WHITE);

  int offset = scroll;
  if (offset > 0) offset -= 16;

  for (int x = offset; x < 128; x += 16) {
    display.drawFastVLine(x, 61, 3, SSD1306_WHITE);
    display.fillRect(x, 61, 8, 2, SSD1306_WHITE);
  }
}

// Palmera de fondo (decorativa), igual que en la versión original
void dibujarPalmera(int x) {
  if (x < -20 || x > 128) return;
  display.drawLine(x, 63, x - 4, 45, SSD1306_WHITE);
  display.drawLine(x + 1, 63, x - 3, 45, SSD1306_WHITE);
  display.drawCircle(x - 4, 45, 6, SSD1306_WHITE);
  display.drawLine(x - 4, 45, x - 14, 42, SSD1306_WHITE);
  display.drawLine(x - 4, 45, x + 6, 42, SSD1306_WHITE);
}

// Rings recolectables: al pasar cerca, se recogen y suman al contador
void dibujarAnillos(long distancia) {
  for (int i = 0; i < NUM_ANILLOS; i++) { 
    if (anilloRecogido[i]) continue;

    int xPantalla = anillosX[i] - distancia + sonicX;
    if (xPantalla > -8 && xPantalla < 128) {
      display.drawCircle(xPantalla, 50, 4, SSD1306_WHITE);
      display.drawCircle(xPantalla, 50, 2, SSD1306_WHITE);

      // Si Sonic pasa muy cerca del ring, se recoge y suma al contador
      if (abs((sonicX + 8) - xPantalla) < 10) {
        anilloRecogido[i] = true;
        contadorRings++;
      }
    }
  }
}
