#include <string.h>
#include "utils.h"

#ifndef CALC_HEADER_
#define CALC_HEADER_

#ifndef MAX_STATE
#	define MAX_STATE	10
#endif

#ifndef MAX_OBSERV
#	define MAX_OBSERV	26
#endif

#ifndef MAX_SEQ
#	define	MAX_SEQ		200
#endif

#ifndef MAX_LINE
#	define MAX_LINE 	256
#endif

static void init_greek ( Data_wrapper* dw_ptr ){
  int            stt_cnt = dw_ptr -> train_ptr -> stt_cnt;
  Greek_letters* gr_ptr  = dw_ptr -> gr_ptr;

  gr_ptr -> alpha   = (double**)malloc( sizeof( double*) * stt_cnt );
  gr_ptr -> beta    = (double**)malloc( sizeof( double*) * stt_cnt );
  gr_ptr -> gamma   = (double**)malloc( sizeof( double*) * stt_cnt );
  gr_ptr -> epsilon = (double**)malloc( sizeof( double*) * stt_cnt );

  for( int i = 0; i < stt_cnt; ++i ){
    gr_ptr -> alpha   [i] = (double*)calloc( MAX_LINE, sizeof( double ) );
  }
  for( int i = 0; i < stt_cnt; ++i ){
    gr_ptr -> beta    [i] = (double*)calloc( MAX_LINE, sizeof( double ) );
  }
  for( int i = 0; i < stt_cnt; ++i ){
    gr_ptr -> gamma   [i] = (double*)calloc( MAX_LINE, sizeof( double ) );
  }
  for( int i = 0; i < stt_cnt; ++i ){
    gr_ptr -> epsilon [i] = (double*)calloc( MAX_LINE, sizeof( double ) );
  }
}

static void* fill_alpha ( void* ptr /* (Data_wrapper*) type */ ){

  // why return type is (void*)?
  // to mute the clang warning on pthread_create.

  Data_wrapper    * dw_ptr   = ((Data_wrapper*)(ptr));
  Parameter_train * pr_ptr   = dw_ptr -> train_ptr;
  Greek_letters   * gr_ptr   = dw_ptr -> gr_ptr;
  HMM             * hmm_ptr  = dw_ptr -> hmm_ptr;
  int               line_cnt = dw_ptr -> cur_line_idx;
  int               stt_cnt  = dw_ptr -> train_ptr -> stt_cnt;

  // reset the matrix
  for( int i = 0; i < stt_cnt; ++i ){
    memset( gr_ptr -> alpha[i], '\0', MAX_LINE);
  }

  // first col. vector
  for( int state_idx = 0; state_idx < stt_cnt; ++state_idx ){
    gr_ptr -> alpha[state_idx][0] = 
      (hmm_ptr -> initial[state_idx]) * ( hmm_ptr -> 
          observation[(pr_ptr->model_data[line_cnt][0]-'A')][state_idx] );
  }

  // recursive col. vector
  for( int observ_idx = 1; 
      (pr_ptr -> model_data[line_cnt][observ_idx] != '\0')&&
      (observ_idx < MAX_LINE);
      ++observ_idx ){


    for( int state_idx_1 = 0; state_idx_1 < stt_cnt; ++state_idx_1 ){
      /*TODO */
      gr_ptr -> alpha[state_idx_1][observ_idx] = 0.0;
      for( int state_idx_2 = 0; state_idx_2 < stt_cnt; ++state_idx_2 ){
        gr_ptr -> alpha[state_idx_1][observ_idx] +=
          gr_ptr  -> alpha[state_idx_2][observ_idx-1] *
          hmm_ptr -> transition[ state_idx_2 ][ state_idx_1 ];
      }
      gr_ptr -> alpha[state_idx_1][observ_idx] *= 
        hmm_ptr -> observation
        [pr_ptr->model_data[line_cnt][observ_idx]-'A'][state_idx_1];
    }

  }

  return NULL;
}


#endif // CALC_HEADER_
