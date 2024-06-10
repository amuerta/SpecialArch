#include "main.h"
#include "config.c"

//  PAGE 0 - 1
//
//  +------------------------+------------------------+
//  |  ## DATA PAGE ##       |  ## CONTROL PAGE ##    |
//  |                        |                        |
//  |  engnie state          |  engine controls       |
//  |  - rpm                 |  - rpm                 |
//  |  - power in V          |  x                     |
//  |  - rotation direction  |  - rotation direction  |
//  |  - engine pin          |  x                     |
//  |                        |                        |
//  |  input handlers        |  ignore option         |
//  |  - button0             |                        |
//  |  - other inputs : {    |                        |
//  |     ...                |                        |
//  |  }                     |                        |
//  |                        |                        |
//  |  other components : {  |  create signal system  |
//  |   ...                  |                        |
//  |  }                     |                        |
//  +------------------------+------------------------+


//  PAGE 2
//
//  +-------------------------------+
//  |  ## Settings PAGE ##          |
//  |                               |
//  |  - Window size: 480p -> 720p  |
//  |  - Update frequency           |
//  |  - Use dark theme             |
//  |  -                            |
//  |                               |
//  +-------------------------------+



//--------------------------------------------------------------//
//                    GLOBAL VARIABLES                          //  
//--------------------------------------------------------------//

#define padding 10
#define padcorn (padding*2)
#define button_size 32
#define label_size 22

const Rectangle content_g = {
  .x = 0+padding,
  .y = 0+padding,
  .width = WIN_WIDTH - padcorn,
  .height = WIN_HEIGHT - padcorn
};

const Rectangle tabs_g = {
  .x = content_g.x,
  .y = content_g.y,
  .width = content_g.width,
  .height = 24
};

const char *tabs[] = {
  "Board monitor", 
  "Board controls",
  "App settings"
};

//--------------------------------------------------------------//
//                    HIDDEN API FUNCTIONS                      //  
//--------------------------------------------------------------//
 
static inline Rectangle after_element(Rectangle r, int height) {
  return (Rectangle) {
    .x = r.x,
    .y = r.y+r.height,
    .width = r.width,
    .height = height
  };
}

static inline Rectangle nexto_element(Rectangle r, int width) {
  return (Rectangle) {
    .x = r.x+r.width,
    .y = r.y,
    .width = width,
    .height = r.height,
  };
}

void MYGuiLabel(Rectangle bounds, const char* text, int text_size) {
  DrawText(text, bounds.x, bounds.y, text_size, hex(0xDFDFDFFF));
}



//--------------------------------------------------------------//
//                    USER INTERFACE COMPONENTS                 //  
//--------------------------------------------------------------//

void tabbar(AppState *s) {
  for(uint i = 0; i < arrlen(tabs); i++)
      if (GuiButton((Rectangle) 
          {
        .x = ((tabs_g.width/arrlen(tabs)) * i) + padding,
        .y = tabs_g.y,
        .width = (tabs_g.width/arrlen(tabs)),
        .height = button_size
      },tabs[i])) 
    
      // IF BUTTON IS PRESSED
      {
        s->active_tab = i;
      }
    
      // IF IDLE AND ACTIVE, MARK WITH UNDERLINE
    else 
      if (i==s->active_tab)
        DrawRectangleRec((Rectangle) {
          .x = ((tabs_g.width/arrlen(tabs)) * i) + padding,
          .y = tabs_g.y,
          .width = (tabs_g.width/arrlen(tabs)),
          .height = 4
        }, hex(0xD0D0D0FF) );
}

void metrics(AppState *s) {
  char* n_buffer[64];

  Rectangle first_label = {
    .x = content_g.x + padding*4,
    .y = content_g.y + button_size + padding*4,
    .width = content_g.width - padcorn - padding*6,
    .height = label_size
  };


  // PROVED TO NOT WORK ON SINGLE FUNC CALL
  // #define METRIC_TO_STRING(ID,FMT,T) ((               \
  //   sprintf(                                    \ 
  //     n_buffer,FMT,(T)(s->I_metrics_table[(ID)]) \
  //   ) > 0)? n_buffer : "null")

  // Board Status

  MYGuiLabel(first_label,"STM32F4 BOARD INFO",label_size);
  GuiLabel(
    (first_label = after_element(first_label,label_size*6)),
    (0 < sprintf(s->formatstr_buffer,
                 // FORMAT
                 "Board found    :  "    "%s" "\n"
                 "Board plug N   :  "    "%s" "\n"
                 "Interface file :  "    "%s" "\n"
                 "Last updated   :  "    "%s" "\n",
                 // VALUES
                  (s->I_metrics_table[MetricsTable_BoardPort] > -1)?
                    "true" : "false", 
                 "null",
                 "null",
                 "never"
    ))?
    s->formatstr_buffer : OVERFLOW_ERROR
  );


  // Engine status display
  char s_rpm[32];
  char s_vlt[32];
  char s_pin[32];
  sprintf(&s_rpm,"%.2f",s->I_metrics_table[MetricsTable_EngineRPM]);
  sprintf(&s_vlt,"%.3f",s->I_metrics_table[MetricsTable_EngineVoltage]);
  sprintf(&s_pin,"%d",(int)s->I_metrics_table[MetricsTable_EnginePin]);

  MYGuiLabel(
    (first_label = after_element(first_label,label_size)),
    "ENGINE METRICS",
    label_size
  );
  GuiLabel(
    (first_label = after_element(first_label,label_size*6)),
    (0 < sprintf(s->formatstr_buffer,
                 // FORMAT
                 "Engine RPM  :  "             "%s" "\n"
                 "Engine Voltage  :  "         "%s" "\n"
                 "Engine rotation vector  :  " "%s" "\n"
                 "Engine pin number  :  "      "%s" "\n",
                 // VALUES
                 s_rpm,
                 s_vlt,
                  (!s->I_metrics_table[MetricsTable_EngineRotation])?
                    "Clockwise" : "CounterClockwise", 
                 s_pin
    ))?
    s->formatstr_buffer : OVERFLOW_ERROR
  );


  // Button status display
  MYGuiLabel(
    (first_label = after_element(first_label,label_size)),
    "INPUT STATUS",
    label_size
  );
  GuiLabel(
    (first_label = after_element(first_label,label_size*4)),
    (0 < sprintf(s->formatstr_buffer,
                 // FORMAT
                 "Format : [ BUTTON_STATE, TIME SINCE LAST INPUT, PIN N ]\n"
                 "Board button  :  "             "[ %s :: %s :: %s ]" "\n"
                 "Board external button  :  "    "[ %s :: %s :: %s ]" "\n",
                 // VALUES
                 (s->I_metrics_table[MetricsTable_Button_Board_IsPressed])?
                 "pressed" : "idle",
                 "undef",//TODO: (MetricsTable_Button_Board_LastInput,"%d",int), 
                 "Has no pin",


                 (s->I_metrics_table[MetricsTable_Button_Extern_IsPressed])?
                 "pressed" : "idle",
                 "undef",// TODO:(MetricsTable_Button_Extern_LastInput,"%d",int), 
                 "undef"// TODO:(MetricsTable_Button_Extern_Pin,"%d",int) 
                 
                 ))?
    s->formatstr_buffer : OVERFLOW_ERROR
  );

  // Monitor related functionality
  first_label.y += label_size*6;
  MYGuiLabel(
    (first_label = after_element(first_label,label_size*2)),
    "MONITOR PANEL ORDERS",
    label_size
  );

  int toggle = (int) (s->sw_options[Software_LogToFile]);
  GuiToggle(
    first_label,
    "Begin logging to file", 
    &toggle);
  first_label.y += label_size ;
  s->sw_options[Software_LogToFile] = (float) toggle;

  #ifndef TODO
  GuiButton(
    (first_label = after_element(first_label,label_size*2)),
    "Select Interface file manually") ;
  #endif

}


static Rectangle controls_engine(AppState *s, Rectangle first_label)
{
  char* n_buffer[64];
  const float orig_x     = first_label.x;
  const float full_width = first_label.width;
  const float half_width = first_label.width / 2.5;


  MYGuiLabel(first_label,"ENGINE CONTROL", label_size);
  {
    // FISRT ELEMENT
    GuiLine((first_label = after_element(first_label,label_size)),0);
    first_label.width = half_width;
    GuiLabel(
      (first_label = after_element(first_label,label_size)),
      "engine rpm"
    );
    sprintf(
      n_buffer,
      "%d / %d",
      HWCFG.maxrpm,
      (int)s->I_request_table[RequestTable_DesiredRPM]
    );
    GuiSlider(
      (first_label = nexto_element(first_label,half_width)),
      "0", n_buffer,
      &(s->I_request_table[RequestTable_DesiredRPM]),
      0, HWCFG.maxrpm
    );
    

    // SECOND ELEMENT
    first_label.y += padding;
    first_label.x = orig_x;
    GuiLabel(
      (first_label = after_element(first_label,label_size)),
      "engine rotation direction"
    );
    GuiToggle(
      (first_label = nexto_element(first_label,half_width)),
      (!s->I_request_table[RequestTable_DesiredRotationDirection])?
      "Clockwise":"Counter Clockwise",
      // &debug
      (int*) &(s->I_request_table[RequestTable_DesiredRotationDirection])
    );

    // THIRD ELEMENT
    first_label.y += padding;
    first_label.x = orig_x;
    
    if(!s->sw_options[Software_Allowdangerous])
      GuiDisable();
  
    GuiLabel(
      (first_label = after_element(first_label,label_size)),
      "engine (set/force) Voltage"
    );
    sprintf(
      n_buffer,
      "%d / %f",
      HWCFG.maxmilivolts,
      s->I_request_table[RequestTable_DesiredMilivolts]
    );
    GuiSlider(
      (first_label = nexto_element(first_label,half_width)),
      "V 0", n_buffer,
      &(s->I_request_table[RequestTable_DesiredMilivolts]),
      0, HWCFG.maxmilivolts
    );
    GuiEnable();
  }
  first_label.x = orig_x;
  first_label.width = full_width;
  return 
      (first_label = after_element(first_label,label_size));
}

static Rectangle controls_led (AppState *s, Rectangle first_label) {
  char* n_buffer[64];

  const float orig_x     = first_label.x;
  const float full_width = first_label.width;
  const float half_width = first_label.width / 2.5;

  first_label.y += label_size;
  MYGuiLabel(first_label,"LED CONTROL", label_size);
  {
    // FISRT ELEMENT
    GuiLine((first_label = after_element(first_label,label_size)),0);
    first_label.width = half_width;
    GuiLabel(
      (first_label = after_element(first_label,label_size)),
      "Light up Board LED(s)"
    );
    GuiToggle(
      (first_label = nexto_element(first_label,half_width)),
      "Light up the night", 
      &(s->I_request_table[RequestTable_DesireLigtLED]));
  
    // SECOND ELEMENT
    first_label.x = orig_x;
    first_label.y += padding;
    GuiLabel(
      (first_label = after_element(first_label,label_size)),
      "Blink the LED"
    );
    sprintf(
      n_buffer,
      "%d bpm  /  %d",
      HWCFG.maxblinkspm,
      (int)s->I_request_table[RequestTable_DesiredBlinkingFreq]
    );
    GuiSlider(
      (first_label = nexto_element(first_label,half_width)),
      "No blinks", n_buffer,
      &(s->I_request_table[RequestTable_DesiredBlinkingFreq]),
      0, HWCFG.maxblinkspm
    );
  }


  first_label.x = orig_x;
  first_label.width = full_width;
  return 
      (first_label = after_element(first_label,label_size));
}



static Rectangle controls_board(AppState *s, Rectangle first_label) {
  char* n_buffer[64];

  const float orig_x     = first_label.x;
  const float full_width = first_label.width;
  const float half_width = first_label.width / 2.5;

  first_label.y += label_size;
  MYGuiLabel(first_label,"BOARD CONTROL", label_size);
  {
    // FISRT ELEMENT
    GuiLine((first_label = after_element(first_label,label_size)),0);
    first_label.width = half_width;
    GuiLabel(
      (first_label = after_element(first_label,label_size)),
      "Ping board"
    );
    GuiButton(
      (first_label = nexto_element(first_label,half_width)),
      "CALL");
  }
  first_label.x = orig_x;
  first_label.width = full_width;
  return 
      (first_label = after_element(first_label,label_size));
}

void controls(AppState *s) {
  Rectangle first_label = {
    .x = content_g.x + padding*4,
    .y = content_g.y + button_size + padding*4,
    .width = content_g.width - padcorn - padding*4,
    .height = label_size
  };
 
  // Render engine controls and update last geometry guideline
  first_label = controls_engine(s,first_label);

  // Render led controls and update last geometry guideline
  first_label = controls_led(s,first_label);

  // Render board controls and update last geometry guideline
  first_label = controls_board(s,first_label);
}

static Rectangle settings_behave(AppState *s, Rectangle first_label)
{
  char* n_buffer[64];
  const float orig_x     = first_label.x;
  const float full_width = first_label.width;
  const float half_width = first_label.width / 2.5;


  MYGuiLabel(first_label,"BEHAVIOUR SETTINGS", label_size);
  {
    // FISRT ELEMENT
    GuiLine((first_label = after_element(first_label,label_size)),0);
    first_label.width = half_width;
    GuiLabel(
      (first_label = after_element(first_label,label_size)),
      "board interface ref/rate"
    );
    sprintf(
      n_buffer,
      "%d / %d",
      SWCFG.maxupdate,
      (int)s->sw_options[Software_RequestSpeed]
    );
    GuiSlider(
      (first_label = nexto_element(first_label,half_width)),
      "0.5", n_buffer,
      &(s->sw_options[Software_RequestSpeed]),
      0.5, SWCFG.maxupdate
    );
    

    // SECOND ELEMENT
    
    // RAW POINTER CONVERSION CAUSES A BUG TO OCCUR
    // THIS INTERMIDIATE BUFFER PREVENTS IT.
    int toggle = (int)(s->sw_options[Software_Allowdangerous]);

    first_label.y += padding;
    first_label.x = orig_x;
    GuiLabel(
      (first_label = after_element(first_label,label_size)),
      "Enable dangerous settings"
    );
    GuiToggle(
      (first_label = nexto_element(first_label,half_width)),
      (!s->sw_options[Software_Allowdangerous])?
      "Disable":"Enable",
      // &debug
      &toggle
    );

    s->sw_options[Software_Allowdangerous] = (float) toggle;
  }
  first_label.x = orig_x;
  first_label.width = full_width;
  return 
      (first_label = after_element(first_label,label_size));
}



static Rectangle settings_appear(AppState *s, Rectangle first_label)
{
  char* n_buffer[64];
  const float orig_x     = first_label.x;
  const float full_width = first_label.width;
  const float half_width = first_label.width / 2.5;

  first_label.y += button_size;
  MYGuiLabel(first_label,"APPEARENCE SETTINGS", label_size);
  {
    // RAW POINTER CONVERSION CAUSES A BUG TO OCCUR
    // THIS INTERMIDIATE POINTER PREVENTS IT.
    int toggle = (int)s->sw_options[Software_UseLightTheme];
    
    // FISRT ELEMENT
    GuiLine((first_label = after_element(first_label,label_size)),0);
    first_label.width = half_width;
    GuiLabel(
      (first_label = after_element(first_label,label_size)),
      "Use light theme"
    );
    GuiToggle(
      (first_label = nexto_element(first_label,half_width)),
      (!s->sw_options[Software_UseLightTheme])?
      "Disable":"Enable",
      // &debug
      &toggle
      // (int*) &(s->sw_options[Software_UseLightTheme])
    );
    
    s->sw_options[Software_UseLightTheme] = (float) toggle;
  }
  first_label.x = orig_x;
  first_label.width = full_width;
  return 
      (first_label = after_element(first_label,label_size));
}

static Rectangle settings_save(AppState *s, Rectangle first_label) {
  first_label.y += label_size*18;
  int pressed = GuiButton(
    (first_label = after_element(first_label,label_size*2)),
    "Save changes") ;

  if(pressed)
    save_config(s);

  return 
      (first_label = after_element(first_label,label_size));
}

void settings(AppState *s) {
  Rectangle first_label = {
    .x = content_g.x + padding*4,
    .y = content_g.y + button_size + padding*4,
    .width = content_g.width - padcorn - padding*4,
    .height = label_size
  };
  

  // settings for program behavior
  first_label = settings_behave(s, first_label);

  // settings for program appearence
  first_label = settings_appear(s, first_label);

  // settings for program appearence
  first_label = settings_save(s, first_label);
}


void react_to_settings(AppState *s) {
  
  // THEME
  // if light:
  //  load_default
  // if not and button color is bright:
  //  load_dark
 

  if (s->sw_options[Software_UseLightTheme]) {
    GuiLoadStyleDefault();
      s->clear_color = (Color) {190,190,190,255};
  }
  else if (GuiGetStyle(LABEL,BASE_COLOR_NORMAL)==0xC9C9C9FF)  {
    GuiLoadStyle("./res/style_dark.rgs");
    s->clear_color = hex(0x303030FF);
  }
  
}


//--------------------------------------------------------------//
//                    UPDATE PROCEDURE                          //  
//--------------------------------------------------------------//
void update(AppState *s) {
  react_to_settings(s);

  GuiPanel(content_g,0);
  tabbar(s);
  
  switch (s->active_tab) {
    case 0: {
      metrics(s);
    } break;

    case 1: {
      controls(s);
    } break;

    case 2: {
      settings(s);
    }
  }
}
