#include "hmm.h"
#include "utils.h"
#include "calc.h"
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

int main( int argc, char** argv )
{
  /* basic argc sanitation */
  if( argc != 4 ){
    printf( "argument count not correct.\n" );
    printf( "usage: test modellist.txt testing_data.txt result.txt\n" );
    exit(1);
  }

  /* basic parameter storage */
  Parameter_test  test;
  load_params( &test , argv, PARAMETER_TEST );
  prep_params( &test , PARAMETER_TEST );
  
  HMM             hmm_list  [ test.model_cnt    ];
  pthread_t       thrd_list [ test.model_cnt    ];
  long double     prob_rnk  [ test.model_cnt    ];
  Viterbi_wrapper wrap_arr  [ test.model_cnt    ];
  Viterbi_letters lett_arr  [ test.model_cnt    ];
  int             belong    [ test.data_vec_cnt ];
  load_models( test.model_list, hmm_list, test.model_cnt );

  for( int i = 0; i < test.model_cnt; ++i ){

    lett_arr[i].delta = (double**)malloc( 
        hmm_list[i].state_num * sizeof( double* ) );
    lett_arr[i].phi   = (int**   )malloc(
        hmm_list[i].state_num * sizeof( int*    ) );

    for( int j = 0; j < hmm_list[i].state_num; ++j ){
      lett_arr[i].delta[j] = (double*)calloc(
          MAX_LINE, sizeof( double ) );
      lett_arr[i].phi  [j] = (int*)   calloc(
          MAX_LINE, sizeof( int    ) );
    }

  }

  for( int i = 0; i < test.model_cnt; ++i ){
    wrap_arr[i].ptr       = &test;
    wrap_arr[i].model_idx = i;
    wrap_arr[i].let_ptr   = lett_arr+i;
  }

  /* main code */
  for( int t = 0; t < test.data_vec_cnt; ++t ){

    for( int i = 0; i < test.model_cnt; ++i ){
      pthread_create( thrd_list+i, 
          NULL, 
          Viterbi, 
          (void*)(wrap_arr+i) );
    }

    for( int i = 0; i < test.model_cnt; ++i ){
      pthread_join( thrd_list[i], prob_rnk+i );
    }

    belong[t] = max_idx( prob_rnk, test.model_cnt );
  }

  /* main code end */


  /* output */
  FILE* fp = open_or_die( test.results , "w");
  Viterbi_OP( fp, belong, &test );
  fclose( fp );

  return 0;
}
