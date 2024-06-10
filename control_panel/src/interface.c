#include "main.h"
#include <time.h>

#define DESERIALIZE_BOOL(BUF,OPTNAME,OPTKEY) do             \
{                                                           \
  if ((option = get_value_from_key((BUF),OPTNAME)) != NULL) \
    s->I_metrics_table[(OPTKEY)] = (strstr(option,"true"))? \
      1 : 0;                                                \
  free(option);                                             \
} while(0);

#define DESERIALIZE_NUMERIC(BUF,OPTNAME,OPTKEY) do          \
{                                                           \
  if ((option = get_value_from_key((BUF),OPTNAME)) != NULL) \
    s->I_metrics_table[(OPTKEY)] =  atoi(option);           \
  free(option);                                             \
} while(0);


#define SERIALIZE_BOOL(BUF,APPSTATE,OPTNAME,OPTKEY) do {      \
  if ((option = get_value_from_key((BUF),OPTNAME)) == NULL) { \
    char* newbuf = append_buffer(                             \
      (BUF),                                                  \
      "\n"OPTNAME":true"                                      \
    ); free((BUF)); (BUF) = newbuf;                           \
  } else {                                                    \
    char* newbuf = modify_buffer_value_at_key(                \
      (BUF),                                                  \
      OPTNAME,                                                \
      ((int)(APPSTATE)->I_metrics_table[(OPTKEY)])?           \
      "true" : "false"                                        \
    ); free((BUF)); (BUF) = newbuf;                           \
  }} while(0);

#define SYNC_PARAMS(OPTKEY_F,OPTKEY_S) s->I_request_table[(OPTKEY_F)]  \
  = s->I_metrics_table[(OPTKEY_S)]

#define SERIALIZE_NUMERIC(BUF,APPSTATE,OPTNAME,OPTKEY,FMT,TYPE) do {   \
  if ((option = get_value_from_key((BUF),OPTNAME)) == NULL) {          \
    char* newbuf = append_buffer(                                      \
      (BUF),                                                           \
      (0<sprintf(                                                      \
         convert_buffer,                                               \
         "\n%s:"FMT, OPTNAME,                                          \
          ((TYPE)(APPSTATE)->I_metrics_table[(OPTKEY)])                \
      )) ? convert_buffer : "0"                                        \
    ); free((BUF)); (BUF) = newbuf;                                    \
  } else {                                                             \
    char* newbuf = modify_buffer_value_at_key(                         \
      (BUF),                                                           \
      OPTNAME,                                                         \
      (0<sprintf(                                                      \
          convert_buffer,                                              \
          FMT,                                                         \
          ((TYPE)(APPSTATE)->I_metrics_table[(OPTKEY)])                \
      ))? convert_buffer : "0"                                         \
    ); free((BUF)); (BUF) = newbuf;                                    \
  }} while(0);


int load_interface(AppState *s) {
  if (!s->output_file)
  {
    s->file_loaded = 0;
    printf("ERROR: failed to obtain interface file\n");
    return 0;
  }

  char* fbuf = fget_file_content(s->output_file);
  if (!fbuf) {
    s->file_loaded = 0;
    printf(
      "ERROR: INTERFACE FILE IS EMPTY, MAKE SURE ITS CONNECTED TO BRIDGE\n"
      "   | INFO: attempting to load default IFile 'stmio.interface'.\n"
    );
    return 0;
  }
  char* option;

  DESERIALIZE_NUMERIC
    (fbuf,  "BOARD.port", MetricsTable_BoardPort);
  DESERIALIZE_NUMERIC
    (fbuf,  "BOARD.lastanswer" , MetricsTable_LastUpdate);
  DESERIALIZE_NUMERIC
    (fbuf,  "BOARD.ENGINE.rpm" , MetricsTable_EngineRPM);
  DESERIALIZE_NUMERIC
    (fbuf,  "BOARD.ENGINE.volt" , MetricsTable_EngineVoltage);
  DESERIALIZE_BOOL
    (fbuf,  "BOARD.ENGINE.rotdir" , MetricsTable_EngineRotation);
  DESERIALIZE_NUMERIC
    (fbuf,  "BOARD.ENGINE.pin" , MetricsTable_EnginePin);
  DESERIALIZE_BOOL
    (fbuf,  "BOARD.BUTTON.NATIVE.ispressed" , MetricsTable_Button_Board_IsPressed);
  DESERIALIZE_NUMERIC
    (fbuf,  "BOARD.BUTTON.NATIVE.lastanswer" , MetricsTable_Button_Board_LastInput);
 
  // sync controls to data from interface once
  if(!s->file_loaded) {
    SYNC_PARAMS(RequestTable_DesiredRPM,MetricsTable_EngineRPM);
    SYNC_PARAMS(RequestTable_DesiredMilivolts,MetricsTable_EngineVoltage);
    SYNC_PARAMS(RequestTable_DesiredRotationDirection,MetricsTable_EngineRotation);
    s->file_loaded = 1;
    printf("INFO:\tMetrics syncronized with ui\n");
  }
  // printf(
  //   "VALUES OF ENGINE.{rpm,volt} = [ %f, %f ]\n",
  //    s->I_metrics_table[MetricsTable_EngineRPM],
  //    s->I_metrics_table[MetricsTable_EngineVoltage]
  //   );

  free(fbuf);
  return 1;
}


void save_to_interface(AppState *s, int interface_avilable) {
  if (!interface_avilable) {
    printf(
      "WARN: where not able to write to the interface "
      "due to previous incidents. Skipping...\n"
    );
    return;
  }

  // load file
  char* fbuf = calloc(1,1);//fget_file_content(s->input_file);
  char* option;
  char  convert_buffer[64];

  SERIALIZE_NUMERIC
    (fbuf,s,"ASK.rpm"        , RequestTable_DesiredRPM, "%i",int);
  SERIALIZE_NUMERIC
    (fbuf,s,"ASK.rotdir"     , RequestTable_DesiredRotationDirection, "%i",int);
  SERIALIZE_NUMERIC
    (fbuf,s,"ASK.volt"       , RequestTable_DesiredMilivolts, "%f",float);
  SERIALIZE_BOOL
    (fbuf,s,"ASK.ledup"      , RequestTable_DesireLigtLED);
  SERIALIZE_NUMERIC
    (fbuf,s,"ASK.ledbinkfreq", RequestTable_DesiredBlinkingFreq,"%i",int);
  SERIALIZE_BOOL
    (fbuf,s,"ASK.callback"   , RequestTable_DesiredBoardAnswer);


  rewind(s->input_file);
  fputs(fbuf,s->input_file);
  free(fbuf);

}

void log_to_file(AppState *s) {
  FILE* f = fopen("stm.log","a+");
  static int tick;

  if (!f) {
    printf("ERROR: for some reason.. failed to open log file!?\n");
    return;
  }

  char* buf[512];
  char* time_str[64];

  // obtain current time
  time_t t = time(NULL);
  struct tm *tm = localtime(&t);
  size_t r = strftime(time_str,arrlen(time_str),"%c",tm);
  if (!r) {
    printf("WARN: failed to log time, string size ecceted maximum\n");
    return;
  }

  // format output buffer
  sprintf(
    buf,
    "+------------------------------------------------+" "\n"
    " LOG REPORT: %s, TICK : %d  \n"
    "\t" "Board Port      : %d" "\n"
    "\t" "Last Update     : %d" "\n"
    "\t" "Engine RPM      : %f" "\n"
    "\t" "Engine Volt     : %f" "\n"
    "\t" "Engine Rotation : %d" "\n"
    "\t" "Engine Pin      : %d" "\n"
    "\t" "Board Button    : %s" "\n"
    "\t" "Extern Button   : %s" "\n"
    "\n\n\n"
    ,
    time_str,tick,
    (int)  s->I_metrics_table[MetricsTable_BoardPort],
    (int)  s->I_metrics_table[MetricsTable_LastUpdate],
    (float)s->I_metrics_table[MetricsTable_EngineRPM],
    (float)s->I_metrics_table[MetricsTable_EngineVoltage],
    (int)  s->I_metrics_table[MetricsTable_EngineRotation],
    (int)  s->I_metrics_table[MetricsTable_EnginePin],
    ((int) s->I_metrics_table[MetricsTable_Button_Board_IsPressed])?
    "true" : "false",
    ((int) s->I_metrics_table[MetricsTable_Button_Extern_IsPressed])?
    "true" : "false"
  );
  tick++;
  fputs(buf,f);
  fclose(f);
}

void update_interface(AppState* s) {
  if (s->update_clock >= 60) {
    s->update_clock = 0;
  }

  int interval_size = (s->sw_options[Software_RequestSpeed]>=1)?
    (int) s->sw_options[Software_RequestSpeed] : 1;


  if (s->update_clock % interval_size == 0)
  {
    int result = load_interface(s);
    save_to_interface(s,result);

    if (s->sw_options[Software_LogToFile])
    {
      log_to_file(s);
    }
  }

  s->update_clock += 1;
}
