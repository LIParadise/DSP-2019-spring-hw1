#ifndef UTILS_HEADER_
#define UTILS_HEADER_

#include <string.h>
#include <stdlib.h>

typedef struct{
  int     it_cnt;      // iteration count
  char*   model_init;  // init model filename
  char*   model_train; // train data filename
  char*   model_OP;    // trained model name
} Parameter_train;

typedef struct{
  char*   model_list;  // list of HMMs to run Viterbi.
  char*   test_data;;  // input sequence of char (39-dim MFCCs)
  char*   results;    // results
} Parameter_test;

typedef enum{
  PARAMETER_TRAIN = 0,
  PARAMETER_TEST
} Dtor_type;

static void load_params( void* ptr,
    char** argv,
    Dtor_type type ){

  if( type == PARAMETER_TRAIN ){

    Parameter_train* pr_ptr = (Parameter_train*)ptr;

    pr_ptr -> it_cnt     = atoi( argv[1] );

    pr_ptr -> model_init = malloc( strlen( argv[2] ) + 1 );
    memcpy( pr_ptr -> model_init, argv[2], strlen(argv[2]) );
    pr_ptr -> model_init [strlen(argv[2])] = '\0';

    pr_ptr -> model_train = malloc( strlen( argv[3] ) + 1 );
    memcpy( pr_ptr -> model_train, argv[3], strlen(argv[3]) );
    pr_ptr -> model_train [strlen(argv[3])] = '\0';

    pr_ptr -> model_OP = malloc( strlen( argv[4] ) + 1 );
    memcpy( pr_ptr -> model_OP, argv[4], strlen(argv[4]) );
    pr_ptr -> model_OP [strlen(argv[4])] = '\0';

  } else if( type == PARAMETER_TEST ){

    Parameter_test* pr_ptr = (Parameter_test*)ptr;

    pr_ptr -> model_list = malloc( strlen( argv[1] ) + 1 );
    memcpy( pr_ptr -> model_list, argv[1], strlen(argv[1]) );
    pr_ptr -> model_list [strlen(argv[1])] = '\0';

    pr_ptr -> test_data = malloc( strlen( argv[2] ) + 1 );
    memcpy( pr_ptr -> test_data, argv[2], strlen(argv[2]) );
    pr_ptr -> test_data [strlen(argv[2])] = '\0';

    pr_ptr -> results = malloc( strlen( argv[3] ) + 1 );
    memcpy( pr_ptr -> results, argv[3], strlen(argv[3]) );
    pr_ptr -> results [strlen(argv[3])] = '\0';

  }

}



static void discard( void* ptr , Dtor_type type){
  if( type == PARAMETER_TRAIN ){
    Parameter_train* pr_ptr = (Parameter_train*)ptr;
    free( pr_ptr -> model_init );
    free( pr_ptr -> model_train );
    free( pr_ptr -> model_OP );
  }else if( type == PARAMETER_TEST ){
    Parameter_test* pr_ptr = (Parameter_test*)ptr;
    free( pr_ptr -> model_list );
    free( pr_ptr -> test_data );
    free( pr_ptr -> results );
  }else {}
}

#endif // UTILS_HEADER_
