// LIBC
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// raylib
// #include "../lib/raylib-5.0_linux_amd64/include/raylib.h"
#include <raylib.h>

// raygui
// Immidiate ui library based of raylib
#define RAYGUI_IMPLEMENTATION
#include "../lib/raygui-4.0/raygui.h"

// comptime include once macro
#pragma once

// GLOBAL macros
#define WIN_WIDTH         600
#define WIN_HEIGHT        800
#define WIN_FPS           60
#define MAX_INPUT_DEVICES 16
#define MAX_METRICS       32
#define MAX_REQUESTS      8
#define CONFIG_OPTIONS    8
#define MAX_CHARS         512
#define OVERFLOW_ERROR "ERROR: metrics format is incorrect (possible buffer overflow)"

#define hex(INT) GetColor((INT))
#define arrlen(ARR) (sizeof(ARR) / sizeof(ARR[0]))

// program is not complete
#define TODO

// types
typedef enum {
  Software_LogToFile      = 0,
  Software_ConnectFile    = 1,
  Software_UpdateSpeed    = 2,
  Software_RequestSpeed   = 3,
  Software_Allowdangerous = 4,
  Software_UseLightTheme  = 5,
} SoftwareOptions;

typedef enum {
  RequestTable_DesiredRPM               = 0,
  RequestTable_DesiredRotationDirection = 1,
  RequestTable_DesiredMilivolts         = 2,
  RequestTable_DesireLigtLED            = 3,
  RequestTable_DesiredBlinkingFreq      = 4,
  RequestTable_DesiredBoardAnswer       = 5,
} RequestTableKeys;

typedef enum {
  MetricsTable_BoardPort               = 0,
  MetricsTable_LastUpdate              = 1,
  MetricsTable_EngineRPM               = 2,
  MetricsTable_EngineVoltage           = 3,
  MetricsTable_EngineRotation          = 4,
  MetricsTable_EnginePin               = 5,
  MetricsTable_Button_Board_IsPressed  = 6,
  MetricsTable_Button_Board_LastInput  = 7,
  MetricsTable_Button_Extern_IsPressed = 8,
  MetricsTable_Button_Extern_LastInput = 9,
  MetricsTable_Button_Extern_Pin       = 10,
} MetricsTableKeys;

typedef struct {
  int   file_loaded;
  uint  update_clock;
  FILE* input_file;
  FILE* output_file;
  int   active_tab;
  char  formatstr_buffer  [MAX_CHARS]; 
  float sw_options        [CONFIG_OPTIONS];
  float I_metrics_table   [MAX_METRICS] ;
  float I_request_table   [MAX_REQUESTS];
  Color clear_color;
} AppState;

typedef struct {
  uint   maxrpm;
  uint   maxmilivolts;
  uint   maxblinkspm;
} HardwareConfig;

typedef struct {
  uint maxupdate;
} SoftwareConfig;

const SoftwareConfig SWCFG = {
  .maxupdate  = 10,
};

const HardwareConfig HWCFG = {
  .maxrpm = 4200,
  .maxmilivolts = 12.0,
  .maxblinkspm = 3000,
};
