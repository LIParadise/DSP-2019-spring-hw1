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
   fclose(fp);
}

static int myload_models( const char *listname, HMM *hmm, const int max_num )
{
   FILE *fp = open_or_die( listname, "r" );

   int count = 0;
   char filename[MAX_LINE] = "";
   while( fscanf(fp, "%s", filename) == 1 ){
      myloadHMM( &hmm[count], filename );
      count ++;

      if( count > max_num ){
         return count;
      }
   }
   fclose(fp);

   return count;
}

int main( int argc, char** argv )
{
  /* basic argc sanitation */
  if( argc != 4 ){
    printf("argument count not correct.\n" );
    printf("use: test modellist.txt testing_data.txt result.txt\n");
    exit(1);
  }

  /* basic parameter storage */
  Parameter_test  test;
  load_params( &test , argv, PARAMETER_TEST );
  prep_params( &test , PARAMETER_TEST );
  HMM             hmm_list  [ test.model_cnt    ];
  myload_models( test.model_list, hmm_list, test.model_cnt );

  pthread_t       thrd_list [ test.model_cnt    ];
  long double     prob_rnk  [ test.model_cnt    ];
  Viterbi_wrapper wrap_arr  [ test.model_cnt    ];
  Viterbi_letters lett_arr  [ test.model_cnt    ];
  int             BELONG    [ test.data_vec_cnt ];
  long double     results   [ test.data_vec_cnt ];

  for( int i = 0; i < test.model_cnt; ++i ){

    lett_arr[i].delta = (long double**)malloc( 
        hmm_list[i].state_num * sizeof( long double* ) );
    lett_arr[i].phi   = (int**        )malloc(
        hmm_list[i].state_num * sizeof( int*    ) );

    for( int j = 0; j < hmm_list[i].state_num; ++j ){
      lett_arr[i].delta[j] = (long double*)calloc(
          MAX_LINE, sizeof( long double ) );

      lett_arr[i].phi  [j] = (int*        )calloc(
          MAX_LINE, sizeof( int         ) );
    }

  }

  for( int i = 0; i < test.model_cnt; ++i ){
    wrap_arr[i].ptr       = &test;
    wrap_arr[i].model_idx = i;
    wrap_arr[i].let_ptr   = lett_arr+i;
    wrap_arr[i].hmm_ptr   = hmm_list+i;
    wrap_arr[i].prob_ptr  = prob_rnk + i;

  }

  /* main code */
  for( int l = 0; l < test.data_vec_cnt; ++l ){

    test.line_idx = l;

    for( int i = 0; i < test.model_cnt; ++i ){
      pthread_create( thrd_list+i, 
          NULL, 
          Viterbi, 
          (void*)(wrap_arr+i) );
    }

    for( int i = 0; i < test.model_cnt; ++i ){
      pthread_join( thrd_list[i], NULL );
    }

    BELONG [l] = max_idx_f( prob_rnk, test.model_cnt );
    results[l] = prob_rnk[ BELONG[l] ];
  }

  /* main code end */

  /* output */
  FILE* fp = open_or_die( test.results , "w");
  Viterbi_OP( fp, BELONG, results, &test );
  fclose( fp );
  /* output end */

  /* garbage clean up*/
  for( int i = 0; i < test.model_cnt; ++i ){

    for( int j = 0; j < hmm_list[i].state_num; ++j ){
      free( lett_arr[i].delta[j] );
      free( lett_arr[i].phi  [j] );
    }
    free ( lett_arr[i].delta );
    free ( lett_arr[i].phi   );
    lett_arr[i].delta = NULL;
    lett_arr[i].phi   = NULL;
    free( hmm_list[i].model_name );
    hmm_list[i]      .model_name = NULL;

  }
  discard( &test, PARAMETER_TEST );

  /* garbage clean up end*/



  return 0;
}
