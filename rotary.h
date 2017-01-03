#ifndef H_ROTARY_H
#include <inttypes.h>
#include "avr-config.h"

int8_t rotary_sample(void);

enum {
  ROTARY_NONE=0,
  ROTARY_RESET=2,
  ROTARY_CW=1,
  ROTARY_CCW=-1
};

void rotary_init(void);
void rotary_start(void);
void rotary_stop(void);

#endif
