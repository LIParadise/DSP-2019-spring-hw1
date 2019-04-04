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
  int              stt_cnt = dw_ptr -> train_ptr -> stt_cnt;
  Greek_letters*   gr_ptr  = dw_ptr -> gr_ptr;
  HMM          *   hmm_ptr = dw_ptr -> hmm_ptr;
  Parameter_train* pr_ptr  = dw_ptr -> train_ptr;

  gr_ptr -> alpha   = (double**) malloc( sizeof( double*)  * stt_cnt );
  gr_ptr -> beta    = (double**) malloc( sizeof( double*)  * stt_cnt );
  gr_ptr -> gamma   = (double**) malloc( sizeof( double*)  * stt_cnt );
  gr_ptr -> epsilon = (double***)malloc( sizeof( double**) * stt_cnt );
  gr_ptr -> gam_end_arr = (double**) malloc( sizeof( double* ) * (pr_ptr -> emt_cnt ) );

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
    gr_ptr -> epsilon [i] = (double**) malloc( sizeof( double*)  * stt_cnt );
    for( int j = 0; j < stt_cnt; ++j ){
      gr_ptr -> epsilon[i][j] = (double*)calloc( MAX_LINE, sizeof( double ) );
    }
  }
  gr_ptr -> gamma_end = (double*)calloc( stt_cnt, sizeof( double ) );
  for( int i = 0; i < pr_ptr -> emt_cnt; ++i ){
    gr_ptr -> gam_end_arr[i] = (double*)calloc(
        stt_cnt, sizeof( double ) );
  }


  gr_ptr -> gamma_arr = (double***)malloc
    ( pr_ptr -> emt_cnt * sizeof( double** ) );

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

  // note:
  // this function is supposed to calc. "gamma" and "gamma_arr"
  // but WITHOUT the normalization
  // that is, we need to normalize the PI array later

  Data_wrapper    * dw_ptr     = ((Data_wrapper*)(ptr));
  Parameter_train * pr_ptr     = dw_ptr -> train_ptr;
  Greek_letters   * gr_ptr     = dw_ptr -> gr_ptr;
  HMM             * hmm_ptr    = dw_ptr -> hmm_ptr;
  int               line_cnt   = dw_ptr -> cur_line_idx;
  int               stt_cnt    = dw_ptr -> train_ptr -> stt_cnt;
  int               obsv_len = 0;
  while( pr_ptr -> model_data[line_cnt][obsv_len] != '\0' ){
    ++ obsv_len ;
  }

  for( int i = 0; i < obsv_len-1; ++i ){
    for( int j = 0; j < stt_cnt; ++j ){
      gr_ptr -> gamma[j][i] += 
        (gr_ptr -> alpha[j][i]) *
        (gr_ptr -> beta[j][i]) *
        obsv_len;
      gr_ptr -> gamma_arr
        [ pr_ptr -> model_data[line_cnt][i] - 'A' ][j][i] +=
        (gr_ptr -> alpha[j][i]) *
          (gr_ptr -> beta[j][i]) *
          obsv_len;
    }
  }

  for( int j = 0; j < stt_cnt; ++j ){

    gr_ptr -> gamma_end[j] +=
      (gr_ptr -> alpha[j][obsv_len-1]) *
      (gr_ptr -> beta [j][obsv_len-1]) *
      obsv_len;

    gr_ptr -> gam_end_arr
      [pr_ptr -> model_data[line_cnt][obsv_len-1]-'A'][j] +=
      gr_ptr -> gamma_end[j];
  }


  return NULL;
}

static void* accm_epsilon ( void* ptr /* (Data_wrapper*) type */ ){

  // note:
  // this function is supposed to calc. "epsilon"
  // but WITHOUT the normalization
  // that is, we need to normalize the PI array later


  Data_wrapper    * dw_ptr     = ((Data_wrapper*)(ptr));
  Parameter_train * pr_ptr     = dw_ptr -> train_ptr;
  Greek_letters   * gr_ptr     = dw_ptr -> gr_ptr;
  HMM             * hmm_ptr    = dw_ptr -> hmm_ptr;
  int               line_cnt   = dw_ptr -> cur_line_idx;
  int               stt_cnt    = dw_ptr -> train_ptr -> stt_cnt;
  int               obsv_len = 0;
  while( pr_ptr -> model_data[line_cnt][obsv_len] != '\0' ){
    ++ obsv_len ;
  }

  for( int stt_idx_1 = 0; stt_idx_1 < stt_cnt; ++ stt_idx_1 ){
    for( int stt_idx_2 = 0; stt_idx_2 < stt_cnt; ++ stt_idx_2 ){
      for( int i = 0; i < obsv_len-1; ++i ){
        /* note the (obsv_len-1) here*/
        gr_ptr -> epsilon[stt_idx_1][stt_idx_2][i] +=
          gr_ptr  -> alpha[ stt_idx_1 ][ i ] *
          gr_ptr  -> beta [ stt_idx_2 ][ i ] *
          hmm_ptr -> transition[ stt_idx_1 ][ stt_idx_2 ] *
          hmm_ptr -> observation
          [(pr_ptr->model_data[line_cnt][0]-'A')][ stt_idx_2 ];
      }
    }
  }

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

static void* calc_pi ( void* ptr /* type (Data_wrapper*) ) */ ){

  Data_wrapper    * dw_ptr    = ((Data_wrapper*)(ptr));
  Parameter_train * pr_ptr    = dw_ptr -> train_ptr;
  Greek_letters   * gr_ptr    = dw_ptr -> gr_ptr;
  HMM             * hmm_ptr   = dw_ptr -> hmm_ptr;
  int               stt_cnt   = dw_ptr -> train_ptr -> stt_cnt;
  int               line_cnt = dw_ptr -> train_ptr -> line_cnt;

  for( int i = 0; i < stt_cnt; ++i ){
    long double sum = 0.0;
    for( int j = 0; j < MAX_LINE; ++j ){
      sum += gr_ptr -> gamma[i][j];
    }
    sum += gr_ptr -> gamma_end[i];
    sum /= gr_ptr -> prob;
    sum /= line_cnt;
    hmm_ptr -> initial[i] = sum;
  }

  return (NULL);
}

static void* calc_A  ( void* ptr /* type (Data_wrapper*) ) */ ){

  Data_wrapper    * dw_ptr    = ((Data_wrapper*)(ptr));
  Parameter_train * pr_ptr    = dw_ptr -> train_ptr;
  Greek_letters   * gr_ptr    = dw_ptr -> gr_ptr;
  HMM             * hmm_ptr   = dw_ptr -> hmm_ptr;
  int               stt_cnt   = dw_ptr -> train_ptr -> stt_cnt;
  int               line_cnt  = dw_ptr -> train_ptr -> line_cnt;

  for( int stt_idx_1 = 0; stt_idx_1 < stt_cnt; ++ stt_idx_1 ){
    long double sum1 = 0.0;
    for( int t = 0; t < MAX_LINE; ++t ){
      sum1 += gr_ptr -> gamma[ stt_idx_1 ][t];
    }
    for( int stt_idx_2 = 0; stt_idx_2 < stt_cnt; ++ stt_idx_2 ){
      long double sum2 = 0.0;
      for( int t = 0; t < MAX_LINE; ++t ){
        sum2 += gr_ptr -> epsilon[stt_idx_1][stt_idx_2][t];
      }
      hmm_ptr -> transition[ stt_idx_1 ][ stt_idx_2 ] = sum2/sum1;
    }
  }

  return (NULL);
}

static void* calc_B  ( void* ptr /* type (Data_wrapper*) ) */ ){

  Data_wrapper    * dw_ptr    = ((Data_wrapper*)(ptr));
  Parameter_train * pr_ptr    = dw_ptr -> train_ptr;
  Greek_letters   * gr_ptr    = dw_ptr -> gr_ptr;
  HMM             * hmm_ptr   = dw_ptr -> hmm_ptr;
  double        *** gamma_arr = gr_ptr -> gamma_arr;
  int               stt_cnt   = dw_ptr -> train_ptr -> stt_cnt;
  int               line_cnt  = dw_ptr -> train_ptr -> line_cnt;
  int               emt_cnt   = pr_ptr -> emt_cnt;

  for( int stt_idx = 0; stt_idx < stt_cnt; ++ stt_idx ){

    long double sum1 = 0.0;
    for( int t = 0; t < MAX_LINE; ++t ){
      sum1 += gr_ptr -> gamma[ stt_idx ][t];
    }
    sum1 += gr_ptr -> gamma_end[ stt_idx ];

    for( int emt_idx = 0; emt_idx < emt_cnt; ++emt_idx ){
      long double sum2 = 0.0;
      for( int t = 0; t < MAX_LINE; ++t ){
        sum2 += gr_ptr -> gamma_arr[emt_idx][ stt_idx ][t];
      }
      sum2 += gr_ptr -> gam_end_arr[emt_idx][stt_idx];
      hmm_ptr -> observation[ emt_idx ][ stt_idx ] = sum2/sum1;
    }

  }

  return (NULL);

}

static void calc_model(
    pthread_t* thrd_1_ptr,
    pthread_t* thrd_2_ptr, 
    pthread_t* thrd_3_ptr,
    Data_wrapper* dw_ptr ){

  pthread_create( thrd_1_ptr, NULL, calc_pi, dw_ptr );
  pthread_create( thrd_2_ptr, NULL, calc_A , dw_ptr );
  pthread_create( thrd_3_ptr, NULL, calc_B , dw_ptr );
  pthread_join  ( *thrd_1_ptr, NULL );
  pthread_join  ( *thrd_2_ptr, NULL );
  pthread_join  ( *thrd_3_ptr, NULL );

}


#endif // CALC_HEADER_
