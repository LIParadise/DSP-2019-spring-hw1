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
  Parameter_train pr;
	HMM             hmm;
  Greek_letters   gr;
  Data_wrapper    dw;
  pthread_t       thrd_1, thrd_2;
  dw.pr_ptr       = &pr;
  dw.gr_ptr       = &gr;
  dw.hmm_ptr      = &hmm;
  load_params( &pr, argv, PARAMETER_TRAIN );
  prep_params( &pr, &hmm, PARAMETER_TRAIN );
  init_greek ( &gr );
  loadHMM( &hmm, pr.model_init );

  /* main code */

  for( int i = 0; i < pr.line_cnt; ++i ){
    dw.cur_line_idx = i;
    pthread_create( &thrd_1  , NULL, fill_alpha  , &dw );
    pthread_create( &thrd_2  , NULL, fill_beta   , &dw );
    pthread_join  ( thrd_1, NULL );
    pthread_join  ( thrd_2, NULL );
    pthread_create( &thrd_1  , NULL, accm_gamma  , &dw );
    pthread_create( &thrd_2  , NULL, accm_epsilon, &dw );
    pthread_join  ( thrd_1, NULL );
    pthread_join  ( thrd_2, NULL );
  }


  /* main code end */


  /* output */
  discard( &pr, PARAMETER_TRAIN );
	dump_models( &hmm, 1 );
  FILE* fp = open_or_die( pr.model_OP , "w");
  dumpHMM( fp, &hmm );
  fclose( fp );

	return 0;
}
