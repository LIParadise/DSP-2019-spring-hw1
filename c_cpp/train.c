#include "hmm.h"
#include "utils.h"
#include <stdlib.h>
#include <math.h>

int main( int argc, char** argv )
{
  if( argc != 5 ){
    printf( "argument count not correct.\n" );
    printf( "usage: ./train iteration model_init.txt seq_model_01.txt model_01.txt\n" );
    exit(1);
  }
  Parameter_train pr;
	HMM       hmms[5];
  load_params( &pr, argv );
	load_models( "modellist.txt", hmms, 5);

  /* main code */


  /* main code end */


  discard( &pr, PARAMETER_TRAIN );
	dump_models( hmms, 5);
	return 0;
}
