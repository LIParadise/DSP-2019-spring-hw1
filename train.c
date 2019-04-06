#include "hmm.h"
#include "utils.h"
#include "calc.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>

static void myloadHMM( HMM *hmm, const char *filename )
{
   int i, j;
   FILE *fp = open_or_die( filename, "r");

   hmm->model_name = (char *)malloc( sizeof(char) * (strlen( filename)+1));
   strcpy( hmm->model_name, filename );

   char token[MAX_LINE] = "";
   while( fscanf( fp, "%s", token ) > 0 )
   {
      if( token[0] == '\0' || token[0] == '\n' ) continue;

      if( strcmp( token, "initial:" ) == 0 ){
         fscanf(fp, "%d", &hmm->state_num );

         for( i = 0 ; i < hmm->state_num ; i++ )
            fscanf(fp, "%lf", &( hmm->initial[i] ) );
      }
      else if( strcmp( token, "transition:" ) == 0 ){
         fscanf(fp, "%d", &hmm->state_num );

         for( i = 0 ; i < hmm->state_num ; i++ )
            for( j = 0 ; j < hmm->state_num ; j++ )
               fscanf(fp, "%lf", &( hmm->transition[i][j] ));
      }
      else if( strcmp( token, "observation:" ) == 0 ){
         fscanf(fp, "%d", &hmm->observ_num );

         for( i = 0 ; i < hmm->observ_num ; i++ )
            for( j = 0 ; j < hmm->state_num ; j++ )
               fscanf(fp, "%lf", &( hmm->observation[i][j]) );
      }
   }

   fclose( fp );
}

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
  pthread_t       thrd_1, thrd_2, thrd_3;
  dw.train_ptr    = &train;
  dw.gr_ptr       = &gr;
  dw.hmm_ptr      = &hmm;
  load_params( &train, argv, PARAMETER_TRAIN );
  myloadHMM  ( &hmm, train.model_init );
  prep_params( &dw, PARAMETER_TRAIN );
  init_greek ( &dw );

  /* main code */

  for( int iteration = 0; 
      iteration < train.it_cnt;
      ++iteration ){

    anti_degen_model( &thrd_1, &thrd_2, &thrd_3, &dw );

    for( int i = 0; i < train.line_cnt; ++i ){
      dw.cur_line_idx = i;
      fill_alpha_and_beta( &thrd_1, &thrd_2, &dw );
      pthread_create( &thrd_1  , NULL, accm_gamma  , &dw );
      pthread_create( &thrd_2  , NULL, accm_epsilon, &dw );
      pthread_join  ( thrd_1, NULL );
      pthread_join  ( thrd_2, NULL );
    }

    calc_model( &thrd_1, &thrd_2, &thrd_3, &dw );

    reset_tmp_data ( &dw );
  }
  normalize_model( &dw );

  /* main code end */


  /* output and cleanup */
  dump_models( &hmm, 1 );
  FILE* fp = open_or_die( train.model_OP , "w");
  dumpHMM( fp, &hmm );
  discard( &dw   , GREEK_LETTERS   );
  discard( &train, PARAMETER_TRAIN );
  free( hmm.model_name );
  hmm.model_name = NULL;
  fclose( fp );

  return 0;
}
