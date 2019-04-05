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

typedef union{
  double value;
  char   arr[sizeof(double)];
}hack;

static void init_greek ( Data_wrapper* dw_ptr ){
  int              stt_cnt = dw_ptr -> train_ptr -> stt_cnt;
  Greek_letters*   gr_ptr  = dw_ptr -> gr_ptr;
  HMM          *   hmm_ptr = dw_ptr -> hmm_ptr;
  Parameter_train* pr_ptr  = dw_ptr -> train_ptr;

  gr_ptr -> prob         = 0.0;
  gr_ptr -> cur_obsv_len = 0;
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

  for( int i = 0; i < (pr_ptr -> emt_cnt) ; ++i ){
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
  int               line_idx = dw_ptr -> cur_line_idx;
  int               stt_cnt  = dw_ptr -> train_ptr -> stt_cnt;

  // reset the matrix
  for( int i = 0; i < stt_cnt; ++i ){
    memset( gr_ptr -> alpha[i], '\0', MAX_LINE);
  }

  // first col. vector
  for( int state_idx = 0; state_idx < stt_cnt; ++state_idx ){
    gr_ptr -> alpha[state_idx][0] = 
      (hmm_ptr -> initial[state_idx]) * ( hmm_ptr -> 
          observation[(pr_ptr->model_data[line_idx][0]-'A')][state_idx] );
  }

  // recursive col. vector
  for( int observ_idx = 1; 
      (pr_ptr -> model_data[line_idx][observ_idx] != '\0')&&
      (observ_idx < MAX_LINE);
      ++observ_idx ){

    // calculating alpha[ stt_idx_1 ][ observ_idx ]
    // sum all stt_idx_2 which we go from stt_idx_2 to stt_idx_1

    for( int stt_idx_1 = 0; stt_idx_1 < stt_cnt; ++stt_idx_1 ){

      gr_ptr -> alpha[stt_idx_1][observ_idx] = 0.0;

      for( int stt_idx_2 = 0; stt_idx_2 < stt_cnt; ++stt_idx_2 ){
        gr_ptr -> alpha[stt_idx_1][observ_idx] +=
          gr_ptr  -> alpha[stt_idx_2][observ_idx-1] *
          hmm_ptr -> transition[ stt_idx_2 ][ stt_idx_1 ];
      }

      gr_ptr -> alpha[stt_idx_1][observ_idx] *= 
        hmm_ptr -> observation
        [pr_ptr->model_data[line_idx][observ_idx]-'A'][stt_idx_1];
    }

  }

  return NULL;
}

static void* fill_beta ( void* ptr /* (Data_wrapper*) type */ ){

  Data_wrapper    * dw_ptr   = ((Data_wrapper*)(ptr));
  Parameter_train * pr_ptr   = dw_ptr -> train_ptr;
  Greek_letters   * gr_ptr   = dw_ptr -> gr_ptr;
  HMM             * hmm_ptr  = dw_ptr -> hmm_ptr;
  int               line_idx = dw_ptr -> cur_line_idx;
  int               stt_cnt  = dw_ptr -> train_ptr -> stt_cnt;
  int               obsv_len = 0;
  while( pr_ptr -> model_data[line_idx][obsv_len] != '\0' ){
    ++ obsv_len ;
  }
  pr_ptr -> total_len    += obsv_len;
  gr_ptr -> cur_obsv_len =  obsv_len;

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

    // beta[stt_idx_2][observ_idx+1] already done for all stt_idx_2
    // calculating ( beta[stt_idx_1][observ_idx] ) from these info

    for( int stt_idx_1 = 0; stt_idx_1 < stt_cnt; ++stt_idx_1 ){

      gr_ptr -> beta[ stt_idx_1][observ_idx] = 0.0;

      for( int stt_idx_2 = 0; stt_idx_2 < stt_cnt; ++stt_idx_2 ){

        gr_ptr -> beta[ stt_idx_1][observ_idx] +=
          hmm_ptr->transition[ stt_idx_1 ][ stt_idx_2 ] *
          hmm_ptr->observation
          [pr_ptr->model_data[line_idx][observ_idx+1]-'A'][stt_idx_2]*
          gr_ptr ->beta[stt_idx_2][observ_idx+1];

      }
    }
  }

  return NULL;
}

static void* accm_gamma ( void* ptr /* (Data_wrapper*) type */ ){

  // note:
  // this function is supposed to calc. "gamma" and "gamma_arr"
  // WITH the normalization
  // where we'll devide the (Greek_letters.prob) in this function
  // that is, we DON'T need to normalize the PI array later

  Data_wrapper    * dw_ptr     = ((Data_wrapper*)(ptr));
  Parameter_train * pr_ptr     = dw_ptr -> train_ptr;
  Greek_letters   * gr_ptr     = dw_ptr -> gr_ptr;
  HMM             * hmm_ptr    = dw_ptr -> hmm_ptr;
  int               line_idx   = dw_ptr -> cur_line_idx;
  int               stt_cnt    = dw_ptr -> train_ptr -> stt_cnt;
  int               obsv_len   = gr_ptr -> cur_obsv_len;

  for( int i = 0; i < obsv_len-1; ++i ){
    for( int j = 0; j < stt_cnt; ++j ){

      long double tmp = 
        (gr_ptr -> alpha[j][i]) *
        (gr_ptr -> beta [j][i]) /
        (gr_ptr -> prob)        *
        obsv_len                ;
      gr_ptr -> gamma[j][i] += tmp;

      gr_ptr -> gamma_arr
        [ pr_ptr -> model_data[line_idx][i] - 'A' ][j][i] += tmp;

    }
  }

  for( int j = 0; j < stt_cnt; ++j ){

    long double tmp = 
      (gr_ptr -> alpha[j][obsv_len-1]) *
      (gr_ptr -> beta [j][obsv_len-1]) /
      (gr_ptr -> prob)                 *
      obsv_len                         ;
    gr_ptr -> gamma_end[j] += tmp;

    gr_ptr -> gam_end_arr
      [pr_ptr -> model_data[line_idx][obsv_len-1]-'A'][j] += tmp;
  }

  return NULL;
}

static void* accm_epsilon ( void* ptr /* (Data_wrapper*) type */ ){

  // note:
  // this function is supposed to calc. "epsilon"
  // WITH the normalization of (P(O|lambda)) and (vector length);
  // that is, we DON'T need to normalize the PI array later

  Data_wrapper    * dw_ptr     = ((Data_wrapper*)(ptr));
  Parameter_train * pr_ptr     = dw_ptr -> train_ptr;
  Greek_letters   * gr_ptr     = dw_ptr -> gr_ptr;
  HMM             * hmm_ptr    = dw_ptr -> hmm_ptr;
  int               line_idx   = dw_ptr -> cur_line_idx;
  int               stt_cnt    = dw_ptr -> train_ptr -> stt_cnt;
  int               obsv_len   = gr_ptr -> cur_obsv_len;

  for( int stt_idx_1 = 0; stt_idx_1 < stt_cnt; ++ stt_idx_1 ){
    for( int stt_idx_2 = 0; stt_idx_2 < stt_cnt; ++ stt_idx_2 ){
      for( int i = 0; i < obsv_len-1; ++i ){

        /* note the (obsv_len-1) here*/

        long double tmp =
          gr_ptr  -> alpha[ stt_idx_1 ][ i ]                     *
          gr_ptr  -> beta [ stt_idx_2 ][ i+1 ]                   *
          hmm_ptr -> transition[ stt_idx_1 ][ stt_idx_2 ]        /
          gr_ptr  -> prob                                        *
          hmm_ptr -> observation
          [(pr_ptr->model_data[line_idx][i+1]-'A')][ stt_idx_2 ] *
          obsv_len                                               ;

        gr_ptr -> epsilon[stt_idx_1][stt_idx_2][i] += tmp;

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
  // update Greek_letters.prob
  Greek_letters *  gr_ptr       = dw_ptr -> gr_ptr;
  int              cur_obsv_len = gr_ptr -> cur_obsv_len;
  gr_ptr        -> prob         = 0.0;
  for( int i = 0; i < dw_ptr -> train_ptr -> stt_cnt; ++i ){
    gr_ptr -> prob += gr_ptr -> alpha[i][cur_obsv_len-1];
  }

}

static void* calc_pi ( void* ptr /* type (Data_wrapper*) ) */ ){

  Data_wrapper    * dw_ptr    = ((Data_wrapper*)(ptr));
  Parameter_train * pr_ptr    = dw_ptr -> train_ptr;
  Greek_letters   * gr_ptr    = dw_ptr -> gr_ptr;
  HMM             * hmm_ptr   = dw_ptr -> hmm_ptr;
  int               stt_cnt   = dw_ptr -> train_ptr -> stt_cnt;
  int               line_cnt  = dw_ptr -> train_ptr -> line_cnt;

  for( int i = 0; i < stt_cnt; ++i ){
    long double sum = 0.0;
    for( int j = 0; j < MAX_LINE; ++j ){
      sum += gr_ptr -> gamma[i][j];
    }
    sum += gr_ptr -> gamma_end[i];
    sum /= pr_ptr -> total_len;
    sum /= ( 
        (pr_ptr -> total_len) /
        ((long double)(pr_ptr -> line_cnt)) );
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

static void normalize_model ( Data_wrapper* dw_ptr ){
  // normalize the hmm.
  HMM*               hmm_ptr = dw_ptr -> hmm_ptr;
  Parameter_train*   pr_ptr  = dw_ptr -> train_ptr;


  // normalize HMM.initial
  double sum = 0.0;
  for( int i = 0; i < pr_ptr -> stt_cnt; ++i ){
    sum += hmm_ptr -> initial[i];
  }
  if( fabs(sum-1.0) >= 0.000001 ){
    int max_idx = 0;
    for( int i = 1; i < pr_ptr -> stt_cnt; ++i ){
      if( hmm_ptr -> initial[i] >= hmm_ptr -> initial[max_idx] )
        max_idx = i;
    }
    hmm_ptr -> initial[max_idx] -= sum-1;
  }

  // normalize HMM.transition;
  for( int row_idx = 0; row_idx < pr_ptr -> stt_cnt; ++ row_idx ){

    sum = 0.0;
    for( int col_idx = 0; col_idx < pr_ptr -> stt_cnt; ++col_idx){
      sum += hmm_ptr -> transition[row_idx][col_idx];
    }
    if( fabs(sum-1.0) >= 0.000001 ){
      int max_idx = 0;
      for( int i = 1; i < pr_ptr -> stt_cnt; ++i ){
        if( hmm_ptr -> transition[row_idx][i] >=
            hmm_ptr -> transition[row_idx][max_idx] ){
          max_idx = i;
        }
      }
      hmm_ptr -> transition[row_idx][max_idx] -= sum-1;
    }

  }

  // normalize HMM.observation;
  for( int col_idx = 0; col_idx < pr_ptr -> stt_cnt; ++ col_idx ){

    sum = 0.0;
    for( int row_idx = 0; row_idx < pr_ptr -> emt_cnt; ++row_idx){
      sum += hmm_ptr -> observation[row_idx][col_idx];
    }
    if( fabs(sum-1.0) >= 0.000001 ){
      int max_idx = 0;
      for( int i = 1; i < pr_ptr -> emt_cnt; ++i ){
        if( hmm_ptr -> observation[i][col_idx] >=
            hmm_ptr -> observation[max_idx][col_idx] ){
          max_idx = i;
        }
      }
      hmm_ptr -> observation[max_idx][col_idx] -= sum-1;
    }

  }
}

static void reset_tmp_data( Data_wrapper* dw_ptr ){
  Greek_letters*    gr_ptr   =   dw_ptr -> gr_ptr;
  Parameter_train*  pr_ptr   =   dw_ptr -> train_ptr;

  gr_ptr -> cur_obsv_len = 0;
  gr_ptr -> prob         = 0.0;
  pr_ptr -> total_len    = 0;

  // Greek_letters: alpha, beta, gamma, gamma_end
  for( int i = 0; i < pr_ptr -> stt_cnt; ++i ){
    memset( gr_ptr -> alpha[i], '\0', MAX_LINE * sizeof( double ) );
    memset( gr_ptr -> beta [i], '\0', MAX_LINE * sizeof( double ) );
    memset( gr_ptr -> gamma[i], '\0', MAX_LINE * sizeof( double ) );
    gr_ptr -> gamma_end[i] = 0.0;
  }

  // Greek_letters: gam_end_arr
  for( int i = 0; i < pr_ptr -> emt_cnt; ++i ){
    memset( 
        gr_ptr -> gam_end_arr[i],
        '\0',
        pr_ptr -> stt_cnt * sizeof( double ) );
  }

  // Greek_letters: gamma_arr
  for( int i = 0; i < pr_ptr -> emt_cnt; ++i ){
    for( int j = 0; j < pr_ptr -> stt_cnt; ++j ){
      memset( 
          gr_ptr -> gamma_arr[i][j],
          '\0', 
          MAX_LINE * sizeof( double ) );
    }
  }

  // Greek_letters: epsilon
  for( int i = 0; i < pr_ptr -> stt_cnt; ++i ){
    for( int j = 0; j < pr_ptr -> stt_cnt; ++j ){
      memset( 
          gr_ptr -> epsilon[i][j], 
          '\0', 
          MAX_LINE*sizeof(double) );
    }
  }

}

void* Viterbi( void* ptr /* of type Viterbi_wrapper */ ){

  Viterbi_wrapper* vw_ptr     = (Viterbi_wrapper*)(ptr);
  int              model_idx  = vw_ptr -> model_idx;
  Parameter_test*  pt_ptr     = vw_ptr -> ptr;
  int              line_idx   = pt_ptr -> line_idx;
  Viterbi_letters* vl_ptr     = vw_ptr -> let_ptr;
  HMM*             hmm_ptr    = vw_ptr -> hmm_ptr;
  int              obsv_len   = 0;
  long double*     prob_ptr   = vw_ptr -> prob_ptr;

  while( pt_ptr -> data_vec_list[line_idx][obsv_len] != '\0' )
    ++ obsv_len;

  for( int i = 0; i < hmm_ptr -> state_num; ++i ){
    memset( vl_ptr -> delta[i], 
        '\0',
        sizeof( long double )*MAX_LINE );
    memset( vl_ptr -> phi  [i], 
        '\0', 
        sizeof( int         )*MAX_LINE );
  }

  // first col of delta
  for( int i = 0; i < hmm_ptr -> state_num; ++i ){
    vl_ptr -> delta[i][0] = hmm_ptr -> initial[i] *
      hmm_ptr->observation
      [pt_ptr->data_vec_list[line_idx][0]-'A'][i];
  }

  // recursive
  for( int t = 1; t < obsv_len; ++ t ){

    for(int stt_idx=0;stt_idx<hmm_ptr->state_num; ++stt_idx ) {

      long double tmp = 0.0;
      int         j   = 0;
      for( int i = 0; i < hmm_ptr -> state_num; ++ i ){
        if( (
              vl_ptr -> delta[i][t-1] *
              hmm_ptr -> transition[i][stt_idx] 
            ) >= tmp ){
          tmp = vl_ptr -> delta[i][t-1] *
            hmm_ptr -> transition[i][stt_idx];
          j = i;
        }
      }
      vl_ptr -> phi     [stt_idx][t] = j;
      vl_ptr -> delta   [stt_idx][t] = 
        vl_ptr  -> delta     [vl_ptr->phi[stt_idx][t]][ t-1 ]   *
        hmm_ptr -> transition[vl_ptr->phi[stt_idx][t]][stt_idx] *
        hmm_ptr -> observation
        [pt_ptr -> data_vec_list[line_idx][t]-'A'][stt_idx] ;
    }

  }

  long double tmp = -0.1;
  int         ret = 0;
  for( int i = 0; i < hmm_ptr -> state_num; ++i ){
    if( vl_ptr -> delta[i][obsv_len-1] >= tmp ){
      tmp = vl_ptr -> delta[i][obsv_len-1];
      ret = i;
    }
  }

  *prob_ptr = vl_ptr -> delta[ret][obsv_len-1];

  return (NULL);

}

int max_idx_f( long double* ptr, int cnt ) {

  int idx = 0;
  for( int i = 0; i < cnt; ++i ){
    if( ptr[i] >= ptr[idx] )
      idx = i;
  }
  return idx;
}

void Viterbi_OP( FILE* fp, 
    int* arr, 
    long double* results, 
    Parameter_test* pt_ptr ){

  int    data_vec_cnt = pt_ptr -> data_vec_cnt;
  for( int i = 0; i < data_vec_cnt; ++i ){
    fprintf( fp, pt_ptr -> mod_name_list[ arr[i] ] );
    fprintf( fp, "\t%.5Le\n", results[i] );
  }

}

#endif // CALC_HEADER_
