/**
 * @file end_credits.c
 * @brief End-credits sequence state machine.
 *
 * Walks through logos, team members, sponsors, and an achievements
 * slideshow, using a simple timed state machine advanced by tick().
 */

#include <stdio.h>
#include <stdbool.h>
#include "end_credits.h"
#include "timer.h"
#include "gfx.h"
#include "project.h"

#include "team.h"
#include "msu_logo.h"
#include "ti_logo.h"
#include "end_credits_logo.h"
#include "cheevos.h"

/**
 * @brief Internal states for the end-credits sequence.
 */
typedef enum {
  ST_LOGO = 0,
  MEMBER_1,
  MEMBER_2,
  MEMBER_3,
  MEMBER_4,
  SPONSOR,
  ECS_TEAM,
  ECS_THANKS_TI,
  FACILITATOR,
  PROFRESSOR,
  ECS_THANKS_MSU,
  ECS_CHEEVOS
} ecs_state_t;

/** Current end-credits state. */
static ecs_state_t g_s;
/** Start time (ms) of current state. */
static uint32_t    g_t0;
/** Dirty flag: true when screen needs a full redraw for current state. */
static bool        g_dirty;

/** Current achievements page index in ECS_CHEEVOS; 0xFF = unset. */
static uint8_t g_ecs_cheevo_page = 0xFF;

/**
 * @brief Transition helper: set new state, reset timer, mark dirty.
 *
 * @param ns Next state to enter.
 */
static void s_goto(ecs_state_t ns){
  g_s     = ns;
  g_t0    = millis();
  g_dirty = true;
}

/**
 * @brief Start the end-credits sequence from the initial logo state.
 */
void end_credits_start(void){
  s_goto(ST_LOGO);
}

/**
 * @brief Advance the end-credits sequence by one frame.
 *
 * This function is non-blocking; it renders or updates only as needed.
 *
 * @return true when the end-credits sequence has fully finished and
 *         the caller may exit to another mode; false while credits
 *         are still ongoing.
 */
bool end_credits_tick(void){
  uint32_t dt = millis() - g_t0;

  switch(g_s){

    case ST_LOGO: {
      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);
        uint8_t x = (uint8_t)((128 - END_CREDITS_LOGO_W) / 2);
        uint8_t y = (uint8_t)((128 - END_CREDITS_LOGO_H) / 2);

        gfx_blit_pal4(x, y,
                      END_CREDITS_LOGO_W, END_CREDITS_LOGO_H,
                      END_CREDITS_LOGO_IDX,
                      END_CREDITS_LOGO_PAL);
      }
      if (dt >= 5000u) s_goto(MEMBER_1);
    } break;

    case MEMBER_1: {
      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);
        gfx_header("SYMAEDCHIT LEO", COL_RED);
        gfx_text2(6, 40, "as: Lead Programmer", COL_WHITE, 1);
      }
      if (dt >= 2000u) s_goto(MEMBER_2);
    } break;

    case MEMBER_2: {
      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);
        gfx_header("ERVIN ALIAJ", COL_BLUE);
        gfx_text2(6, 40, "as: PCB Designer", COL_WHITE, 1);
      }
      if (dt >= 2000u) s_goto(MEMBER_3);
    } break;

    case MEMBER_3: {
      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);
        gfx_header("ANDREW PEREZ", COL_GREEN);
        gfx_text2(6, 40, "as: Power Designer", COL_WHITE, 1);
      }
      if (dt >= 2000u) s_goto(MEMBER_4);
    } break;

    case MEMBER_4: {
      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);
        gfx_header("PRATIJIT PODDER", COL_PURPLE);
        gfx_text2(6, 40, "as: Analog Filterings", COL_WHITE, 1);
      }
      if (dt >= 2000u) s_goto(ECS_TEAM);
    } break;

    case ECS_TEAM: {
      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);
        gfx_header("TEAM #12", COL_WHITE);
        gfx_bar(0, 18, 128, 1, COL_DKGRAY);

        uint8_t x = (uint8_t)((128 - TEAM_W) / 2);
        uint8_t y = (uint8_t)((128 - TEAM_H) / 2);

        gfx_blit_pal4(x, y,
                      TEAM_W, TEAM_H,
                      TEAM_IDX,
                      TEAM_PAL);
      }
      gfx_text2(6, 20, "Leo", COL_RED, 1);
      gfx_text2(35, 120, "Ervin", COL_BLUE, 1);
      gfx_text2(55, 20, "Pratijit", COL_GREEN, 1);
      gfx_text2(85, 120, "Andrew", COL_PURPLE, 1);
      if (dt >=3000u) s_goto(SPONSOR);
    } break;

    case SPONSOR: {
      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);
        gfx_header("GERASIMOS MADALVANOS", COL_RED);
        gfx_text2(6, 40, "as: TI Sponsor", COL_WHITE, 1);
      }
      if (dt >= 3000u) s_goto(ECS_THANKS_TI);
    } break;

    case ECS_THANKS_TI: {
      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);
        gfx_header("THANKS TO:", COL_WHITE);
        gfx_bar(0, 18, 128, 1, COL_DKGRAY);

        uint8_t x = (uint8_t)((128 - TI_LOGO_W) / 2);
        uint8_t y = (uint8_t)((128 - TI_LOGO_H) / 2);

        gfx_blit_pal4(x, y,
                      TI_LOGO_W, TI_LOGO_H,
                      TI_LOGO_IDX,
                      TI_LOGO_PAL);
      }
      if (dt >= 3000u) s_goto(FACILITATOR);
    } break;

    case FACILITATOR: {
      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);
        gfx_header("JOYDEEP MITRA", COL_RED);
        gfx_text2(6, 40, "as: the Facilitator", COL_WHITE, 1);
      }
      if (dt >= 3000u) s_goto(PROFRESSOR);
    } break;

    case PROFRESSOR: {
      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);
        gfx_header("SUBIR BISWAS", COL_RED);
        gfx_text2(6, 40, "as: the Profressor", COL_WHITE, 1);
      }
      if (dt >= 3000u) s_goto(ECS_THANKS_MSU);
    } break;

    case ECS_THANKS_MSU: {
      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);
        gfx_header("THANKS TO:", COL_WHITE);
        gfx_bar(0, 18, 128, 1, COL_DKGRAY);

        uint8_t x = (uint8_t)((128 - MSU_LOGO_W) / 2);
        uint8_t y = (uint8_t)(((128 - MSU_LOGO_H) / 2) + 10);

        gfx_blit_pal4(x, y,
                      MSU_LOGO_W, MSU_LOGO_H,
                      MSU_LOGO_IDX,
                      MSU_LOGO_PAL);
      }
      if (dt >= 3000u){
        s_goto(ECS_CHEEVOS);
      }
    } break;

    case ECS_CHEEVOS: {
      if (g_dirty) {
        g_dirty = false;
      }

      // Decide which page we *want* to show based on dt
      uint8_t page;
      if (dt < 3000u) {
        page = 0;
      } else if (dt < 6000u) {
        page = 1;
      } else if (dt < 9000u) {
        page = 2;
      } else if (dt < 12000u) {
        page = 3;
      } else {
        // After 20s total, reset tracker and finish credits
        g_ecs_cheevo_page = 0xFF;
        return true;
      }

      // Only redraw when we *change* page or on first entry
      if (page != g_ecs_cheevo_page) {
        g_ecs_cheevo_page = page;
        gfx_clear(COL_BLACK);
        cheevos_draw_panel_page(page);
      }
    } break;
  }
  return false;
}
