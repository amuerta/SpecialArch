#include "main.h"
#include "parser.c"

int isspace(char c) {
  if (c == ' ' || c == '\n' || c == '\t')
    return 1;
  return 0;
}

char *ltrim(char *s) {
    while(isspace(*s)) s++;
    return s;
}
char *rtrim(char *s) {
    char* back = s + strlen(s);
    while(isspace(*--back));
    *(back+1) = '\0';
    return s;
}

char *trim(char *s) {
    return rtrim(ltrim(s)); 
}


void load_config(AppState* s) {
 
  #define DESERIALIZE_BOOL(BUF,OPTNAME,OPTKEY) do             \
  {                                                           \
    if ((option = get_value_from_key((BUF),OPTNAME)) != NULL) \
      s->sw_options[(OPTKEY)] = (strstr(option,"true"))?      \
        1 : 0;                                                \
    free(option);                                             \
  } while(0);

  #define DESERIALIZE_NUMERIC(BUF,OPTNAME,OPTKEY) do          \
  {                                                           \
    if ((option = get_value_from_key((BUF),OPTNAME)) != NULL) \
      s->sw_options[(OPTKEY)] =  atoi(option);                \
    free(option);                                             \
  } while(0);

  #define DESERIALIZE_FILE(BUF,OPTNAME,FILE,PARAM)        do    \
  {                                                             \
    if ((option = get_value_from_key((BUF),OPTNAME)) != NULL)   \
      (FILE) = fopen(trim(option),PARAM);                       \
                                                                \
    free(option);                                               \
  } while(0);

  char* fbuf = get_file_content("stmpanel.conf");
  if (!fbuf) {
    printf(
      "WARN: CONFIG FILE NOT FOUND OR EMPTY\n"
      "   | INFO: using default config preset.\n"
    );
    return;
  }
 
  char *option = 0;

  DESERIALIZE_FILE
    (fbuf,  "CONFIG.OutputIfaceFilePath"  ,s->output_file, "r");
  DESERIALIZE_FILE
    (fbuf,  "CONFIG.InputIfaceFilePath"   ,s->input_file , "w");
  DESERIALIZE_BOOL
    (fbuf,  "CONFIG.UseLightTheme"        , Software_UseLightTheme  );
  DESERIALIZE_NUMERIC
    (fbuf,  "CONFIG.InterfaceRequestSpeed", Software_RequestSpeed   );
  DESERIALIZE_BOOL
    (fbuf,  "CONFIG.AllowDangerous"       , Software_Allowdangerous );
  DESERIALIZE_BOOL
    (fbuf,  "CONFIG.LogToFile"   , Software_LogToFile      );


  printf("HELLO?\n");
  printf("DESERIALIZE_BOOL :: Software_LogToFile = %d\n",(int)s->sw_options[Software_LogToFile]);

  free(fbuf);
}

void save_config(AppState *s) {
  // macros are evil that everyone has to commit
  //      -me, June 8 of 2024

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
        ((int)(APPSTATE)->sw_options[(OPTKEY)])?                \
        "true" : "false"                                        \
      ); free((BUF)); (BUF) = newbuf;                           \
    }} while(0);

  #define SERIALIZE_NUMERIC(BUF,APPSTATE,OPTNAME,OPTKEY,FMT,TYPE) do {   \
    if ((option = get_value_from_key((BUF),OPTNAME)) == NULL) {          \
      char* newbuf = append_buffer(                                      \
        (BUF),                                                           \
        (0<sprintf(                                                      \
           convert_buffer,                                               \
           "\n%s:"FMT, OPTNAME,                                          \
            ((TYPE)(APPSTATE)->sw_options[(OPTKEY)])                     \
        )) ? convert_buffer : "0"                                        \
      ); free((BUF)); (BUF) = newbuf;                                    \
    } else {                                                             \
      char* newbuf = modify_buffer_value_at_key(                         \
        (BUF),                                                           \
        OPTNAME,                                                         \
        (0<sprintf(                                                      \
            convert_buffer,                                              \
            FMT,                                                         \
            ((TYPE)(APPSTATE)->sw_options[(OPTKEY)])                     \
        ))? convert_buffer : "0"                                         \
      ); free((BUF)); (BUF) = newbuf;                                    \
    }} while(0);

 


  const char* file = "stmpanel.conf";
  char* fbuf = get_file_content(file);
  char convert_buffer[64];

  if (!fbuf) {
    printf("WARN: failed to save config from runtime!\n");
    return;
  }

  char* option;

  SERIALIZE_BOOL
    (fbuf,s,"CONFIG.AllowDangerous"       ,Software_Allowdangerous);
  SERIALIZE_BOOL
    (fbuf,s,"CONFIG.UseLightTheme"        ,Software_UseLightTheme );
  SERIALIZE_BOOL
    (fbuf,s,"CONFIG.LogToFile"            ,Software_LogToFile     );
  SERIALIZE_NUMERIC
    (fbuf,s,"CONFIG.InterfaceRequestSpeed",Software_RequestSpeed  ,"%i",int);
 
  // a bug happens if line is terminated by EOF
  // when DESERIALIZING it.
  // So i forcefully insert newline to ductape it.
  char*nfbuf = append_buffer(fbuf," \n");
  free(fbuf);
  fbuf = nfbuf;

  FILE* f = fopen(file,"w+");
  if (!f)
    printf("ERROR: failed to open file for saving, even tho its valid!\n");

  fputs(fbuf,f);
  fclose(f);
  free(fbuf);
}
