#ifndef UTILS_HEADER_
#define UTILS_HEADER_

#include <string.h>
#include <stdlib.h>

typedef struct{
  int     it_cnt;      // iteration count
  int     line_cnt;    // total lines in the "model_train" file

  char*   model_init;  // init model filename
  char*   model_train; // train data filename
  char*   model_OP;    // trained model name

  char**  model_data;  // data, i.e. many lines of char
} Parameter_train;

typedef struct{
  char*   model_list;  // list of HMMs to run Viterbi.
  char*   test_data;;  // input sequence of char (39-dim MFCCs)
  char*   results;    // results
} Parameter_test;

typedef enum{
  PARAMETER_TRAIN = 0,
  PARAMETER_TEST
} Pram_type;

static void prep_params(
    void* ptr,
    HMM*  hmm_ptr,
    Pram_type type ){
  if( type == PARAMETER_TRAIN ){
    
    // get line_cnt;
    Parameter_train* pr_ptr = (Parameter_train*)ptr;
    FILE* fp  = open_or_die( pr_ptr -> model_train , "r" );
    char  buf   [MAX_LINE];
    pr_ptr -> line_cnt = 0;
    while (fgets(buf, MAX_LINE, fp) != NULL)
    {
      pr_ptr -> line_cnt++;
    }

    // get the data;
    pr_ptr -> model_data = (char**)
      malloc(sizeof(char*)*(pr_ptr -> line_cnt) );
    for( int i = 0; i < pr_ptr -> line_cnt; ++i ){
      pr_ptr -> model_data[i] = (char*)calloc( MAX_LINE, sizeof(char) );
    }
    rewind(fp);
    for( int i = 0; i < pr_ptr->line_cnt; ++i ){
      fgets(pr_ptr->model_data[i], MAX_LINE, fp) ;
    }

    fclose( fp );

  }
}

static void load_params( void* ptr,
    char** argv,
    Pram_type type ){

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



static void discard( void* ptr , Pram_type type){
  if( type == PARAMETER_TRAIN ){
    Parameter_train* pr_ptr = (Parameter_train*)ptr;
    free( pr_ptr -> model_init );
    free( pr_ptr -> model_train );
    free( pr_ptr -> model_OP );
    for( int i = 0; i < pr_ptr -> line_cnt ; ++i ){
      free( pr_ptr -> model_data[i] );
    }
    free( pr_ptr -> model_data );
    
    pr_ptr -> model_init  = NULL;
    pr_ptr -> model_train = NULL;
    pr_ptr -> model_OP    = NULL;
    pr_ptr -> model_data  = NULL;

  }else if( type == PARAMETER_TEST ){
    Parameter_test* pr_ptr = (Parameter_test*)ptr;
    free( pr_ptr -> model_list );
    free( pr_ptr -> test_data );
    free( pr_ptr -> results );

    pr_ptr -> model_list  = NULL;
    pr_ptr -> test_data   = NULL;
    pr_ptr -> results     = NULL;
  }else {}
}

#endif // UTILS_HEADER_
