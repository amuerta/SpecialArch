#include "main.h"

void deinit(AppState* s) {
  if (s->input_file)
    fclose(s->input_file);
  if (s->output_file)
    fclose(s->output_file);
}
