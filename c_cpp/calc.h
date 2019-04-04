#include <string.h>
#include <pthread.h>
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
  HMM          * hmm_ptr = dw_ptr -> hmm_ptr;

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


  gr_ptr -> gamma_arr = (double***)malloc
    ( hmm_ptr -> observ_num * sizeof( double** ) );

  for( int i = 0; i < hmm_ptr -> observ_num; ++i ){
    gr_ptr -> gamma_arr[i] = (double**)malloc( sizeof( double*) * stt_cnt );
    for( int j = 0; j < stt_cnt; ++j ){
      gr_ptr -> gamma_arr[i][j] = (double*)calloc( MAX_LINE, sizeof( double ) );
    }
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


    for( int stt_idx_1 = 0; stt_idx_1 < stt_cnt; ++stt_idx_1 ){
      gr_ptr -> alpha[stt_idx_1][observ_idx] = 0.0;
      for( int stt_idx_2 = 0; stt_idx_2 < stt_cnt; ++stt_idx_2 ){
        gr_ptr -> alpha[stt_idx_1][observ_idx] +=
          gr_ptr  -> alpha[stt_idx_2][observ_idx-1] *
          hmm_ptr -> transition[ stt_idx_2 ][ stt_idx_1 ];
      }
      gr_ptr -> alpha[stt_idx_1][observ_idx] *= 
        hmm_ptr -> observation
        [pr_ptr->model_data[line_cnt][observ_idx]-'A'][stt_idx_1];
    }

  }

  return NULL;
}

static void* fill_beta ( void* ptr /* (Data_wrapper*) type */ ){

  Data_wrapper    * dw_ptr   = ((Data_wrapper*)(ptr));
  Parameter_train * pr_ptr   = dw_ptr -> train_ptr;
  Greek_letters   * gr_ptr   = dw_ptr -> gr_ptr;
  HMM             * hmm_ptr  = dw_ptr -> hmm_ptr;
  int               line_cnt = dw_ptr -> cur_line_idx;
  int               stt_cnt  = dw_ptr -> train_ptr -> stt_cnt;
  int               obsv_len = 0;
  while( pr_ptr -> model_data[line_cnt][obsv_len] != '\0' ){
    ++ obsv_len ;
  }

  // reset the matrix
  for( int i = 0; i < stt_cnt; ++i ){
    memset( gr_ptr -> beta[i], '\0', MAX_LINE);
  }

  // initial condition, i.e. last col vector of the beta matrix
  for( int state_idx = 0; state_idx < stt_cnt; ++state_idx ){
    gr_ptr -> beta[state_idx][obsv_len-1] = 1;
  }

  // recursive col vector 
  for( int observ_idx = obsv_len-2;
      observ_idx >= 0;
      --observ_idx ){


    for( int stt_idx_1 = 0; stt_idx_1 < stt_cnt; ++stt_idx_1 ){

      gr_ptr -> beta[ stt_idx_1][observ_idx] = 0.0;

      for( int stt_idx_2 = 0; stt_idx_2 < stt_cnt; ++stt_idx_2 ){

        gr_ptr -> beta[ stt_idx_1][observ_idx] +=
          hmm_ptr->transition[ stt_idx_1 ][ stt_idx_2 ] *
          hmm_ptr->observation
          [pr_ptr->model_data[line_cnt][observ_idx+1]-'A'][stt_idx_2]*
          gr_ptr ->beta[stt_idx_2][observ_idx+1];

      }
    }
  }

  // update Greek_letters.prob
  gr_ptr -> prob = 0.0;
  for( int stt_idx = 0; stt_idx < stt_cnt; ++ stt_idx ){
    gr_ptr -> prob += gr_ptr -> alpha[ stt_idx ][ obsv_len-1 ];
  }

  return NULL;
}

static void* accm_gamma ( void* ptr /* (Data_wrapper*) type */ ){
  /* TODO */
  
  return NULL;
}

static void* accm_epsilon ( void* ptr /* (Data_wrapper*) type */ ){
  /* TODO */
  return NULL;
}

inline static void fill_alpha_and_beta( 
  pthread_t* thrd_1, pthread_t* thrd_2, Data_wrapper* dw_ptr ){
  // since fill_alpha and fill_beta have some data dependency,
  // Greek_letters.prob, to be exact,
  // we could create a wrapper function for it.
  pthread_create( thrd_1  , NULL, fill_alpha , (void*) dw_ptr );
  pthread_create( thrd_2  , NULL, fill_beta  , (void*) dw_ptr );
  pthread_join  ( *thrd_1, NULL );
  pthread_join  ( *thrd_2, NULL );
}


#endif // CALC_HEADER_
