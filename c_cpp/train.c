#include "hmm.h"
#include "utils.h"
#include <stdlib.h>
#include <math.h>

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
  load_params( &pr, argv );
  loadHMM( &hmm, pr -> model_init );

  /* main code */


  /* main code end */


  discard( &pr, PARAMETER_TRAIN );
	dump_models( &hmm, 1 );
  FILE* fp = open_or_die( pr -> model_OP , "w");
  dumpHMM( fp, &hmm );
  fclose( fp );

	return 0;
}
