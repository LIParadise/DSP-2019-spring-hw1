#include "hmm.h"
#include "utils.h"
#include "calc.h"
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

int main( int argc, char** argv )
{
  /* basic argc sanitation */
  if( argc != 5 ){
    printf( "argument count not correct.\n" );
    printf( "usage: ./train iteration model_init.txt seq_model_01.txt model_01.txt\n" );
    exit(1);
  }

  /* basic parameter storage */
  Parameter_train train;
	HMM             hmm;
  Greek_letters   gr;
  Data_wrapper    dw;
  pthread_t       thrd_1, thrd_2;
  dw.train_ptr    = &train;
  dw.gr_ptr       = &gr;
  dw.hmm_ptr      = &hmm;
  load_params( &train, argv, PARAMETER_TRAIN );
  loadHMM    ( &hmm, train.model_init );
  prep_params( &dw, PARAMETER_TRAIN );
  init_greek ( &dw );

  /* main code */

  for( int i = 0; i < train.line_cnt; ++i ){
    dw.cur_line_idx = i;
    fill_alpha_and_beta( &thrd_1, &thrd_2, &dw );
    pthread_create( &thrd_1  , NULL, accm_gamma  , &dw );
    pthread_create( &thrd_2  , NULL, accm_epsilon, &dw );
    pthread_join  ( thrd_1, NULL );
    pthread_join  ( thrd_2, NULL );
  }


  /* main code end */


  /* output */
	dump_models( &hmm, 1 );
  FILE* fp = open_or_die( train.model_OP , "w");
  dumpHMM( fp, &hmm );
  discard( &train, PARAMETER_TRAIN );
  fclose( fp );

	return 0;
}
