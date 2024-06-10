#include "ui.c"
#include "interface.c"
#include "deinit.c"

//-Forward Define-//
void test_pasrser();
void test_load   ();
void test_save   ();

//----DEBUG------//
// #define DEBUG //


//--------------------------------------------------------------//
//                    MAIN FUNCTIONS                            //  
//--------------------------------------------------------------//

 
#ifdef DEBUG
  int main(void ){
    test_pasrser();
    test_load();
    test_save();
    return 0;
  }
#endif



#ifndef DEBUG
  int main(void) {
    AppState app = {};
    load_config(&app);

    InitWindow(WIN_WIDTH,WIN_HEIGHT,"STM32 Remote");
    SetTargetFPS(60);
    while(!WindowShouldClose())
    {
      BeginDrawing();
      ClearBackground(app.clear_color);
      update(&app);
    
      update_interface(&app);

      EndDrawing();
    }
    CloseWindow();
    deinit(&app);

  }
#endif 


//--------------------------------------------------------------//
//                          UNIT TESTS                          //  
//--------------------------------------------------------------//

#define PADDING() do {        \
  printf("\n");               \
  for(uint i = 0; i < 15;i++) \
    printf("-");              \
  printf("\n");               \
} while(0)

void test_save() {
  PADDING();
  AppState app = {};
  app.sw_options[Software_RequestSpeed] = 10;
  save_config(&app);
}

void test_load() {
  PADDING();
  AppState app = {};
  load_config(&app);
  printf("app.sw_options[UseLightTheme] : %d\n",
         (int)app.sw_options[Software_UseLightTheme]
  );
  printf("app.sw_options[RequestSpeed] : %d\n",
         (int)app.sw_options[Software_RequestSpeed]
  );
  printf("app.InterfaceFiles : [%p :: %p]\n",
         app.input_file, app.output_file
  );
}

void test_pasrser() {
   PADDING();
   char* str = get_file_content("./test/template");
   char *v = get_value_from_key(str,"shrimp");
   printf("VALUE: %s\n\n\n",v);
  
   printf("STR : %s\n\n\n",str);
   char* new = modify_buffer_value_at_key(str,"shrimp","BIH SHRIMP!");
   
   printf("MOD BUFFER: {%s}",new);
   if (new)
     free(new);
   free(v);
   free(str);
}
