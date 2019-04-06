#ifndef UTILS_HEADER_
#define UTILS_HEADER_

#include <string.h>
#include <stdlib.h>

typedef struct Greek_letters{

  // remark:
  // the matrices gamma and its derivatives and epsilon
  // are normalized in regard to observation length
  // in order to accommodate for different length of observation


  double ** alpha      ;  // alpha [state count][observation index]
  double ** beta       ;  // beta  [state count][observation index]
  double ** gamma      ;  // gamma [state count][observation index-1]
  double *  gamma_end  ;  // col vector of length (state count)

  double *** epsilon   ;  // [current state], max = (state count)
  //                      // [next state], max = (state count)
  //                      // [observation index]

  double *** gamma_arr ;  // gamma regarding a specific observation
  double ** gam_end_arr;  // gamma_end of some specific observation

  long double prob     ;  // prob( O | lambda );
  //                      // sum of last col of alpha matrix.
  //                      // s.t. we don't have to calculate this
  //                      // value for each (observation index)
  //                      // while accumulating gamma and epsilon.
  //
  int     cur_obsv_len ;


} Greek_letters;

typedef struct{
  long double ** delta;
  int         ** phi  ; 
} Viterbi_letters;


typedef struct{
  int     it_cnt;      // iteration count
  int     line_cnt;    // total lines in the "model_train" file
  int     stt_cnt;     // how many # of hidden states in the HMM.
  int     emt_cnt;     // how many # of possible emissions.
  //                      (since it's HMM).
  size_t  total_len;   // total observed length 
  //                   // for normalizing gamma

  char*   model_init;  // init model filename
  char*   model_train; // train data filename
  char*   model_OP;    // trained model name

  char**  model_data;  // data, i.e. many lines of char

} Parameter_train;

typedef struct{
  char*   model_list    ;  // list of HMMs to run Viterbi.
  char*   test_data     ;  // input sequence of char (39-dim MFCCs)
  char*   results       ;  // results

  char**  mod_name_list ;
  char**  data_vec_list ;

  int     model_cnt     ;

  int     data_vec_cnt  ;

  int     line_idx      ;
} Parameter_test;

typedef struct {
  int               model_idx;
  Parameter_test*   ptr;
  Viterbi_letters*  let_ptr;
  HMM*              hmm_ptr;
  long double*      prob_ptr;

} Viterbi_wrapper;

typedef struct{
  Parameter_train* train_ptr;
  Parameter_train* test_ptr;
  Greek_letters*   gr_ptr;
  HMM*             hmm_ptr;
  int              cur_line_idx;  // current line index
} Data_wrapper;

typedef enum{
  PARAMETER_TRAIN = 0,
  PARAMETER_TEST     ,
  GREEK_LETTERS      ,
} Pram_type;

static void prep_params(
    void* ptr, // pointer of type Data_wrapper or Parameter_test
    Pram_type type ){

  if( type == PARAMETER_TRAIN ){

    Parameter_train* pr_ptr = ((Data_wrapper*)ptr) -> train_ptr;

    // get (# of states) and (# of possible emissions from a state)
    // shall be called only after HMM is loaded.
    pr_ptr -> stt_cnt = ((Data_wrapper*)ptr) -> hmm_ptr -> state_num;
    pr_ptr -> emt_cnt = ((Data_wrapper*)ptr) -> hmm_ptr -> observ_num;
    pr_ptr -> total_len = 0;

    // get line_cnt;
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

      // sanitize the change line symbols
      char *pos, *min_pos;
      min_pos = NULL;
      if ((pos=strchr(pr_ptr -> model_data[i], '\n')) != NULL)
        min_pos = pos;
      if ((pos=strchr(pr_ptr -> model_data[i], '\r')) != NULL)
        if( pos < min_pos )
          min_pos = pos;
      if( min_pos != NULL )
        memset( min_pos, '\0', 
            MAX_LINE - ( min_pos - pr_ptr -> model_data[i]) );
    }

    fclose( fp );

  }else if( type == PARAMETER_TEST ){

    Parameter_test * pt_ptr = ((Parameter_test*)ptr) ;

    // get line_cnt;
    FILE* fp  = open_or_die( pt_ptr -> model_list, "r" );
    char  buf   [MAX_LINE];
    pt_ptr -> model_cnt = 0;
    while (fgets(buf, MAX_LINE, fp) != NULL)
    {
      pt_ptr -> model_cnt++;
    }
    rewind(fp);
    pt_ptr -> mod_name_list = (char**)malloc(
        sizeof( char* ) * pt_ptr -> model_cnt );
    for( int i = 0; i < pt_ptr -> model_cnt; ++i ){
      pt_ptr -> mod_name_list[i] = (char*)malloc(
          sizeof(char) * MAX_LINE );
    }

    for( int i = 0; i < pt_ptr->model_cnt; ++i ){

      fgets(pt_ptr->mod_name_list[i], MAX_LINE, fp) ;

      // sanitize the change line symbols
      char *pos, *min_pos;
      min_pos = NULL;
      if ((pos=strchr(pt_ptr -> mod_name_list[i], '\n')) != NULL)
        min_pos = pos;
      if ((pos=strchr(pt_ptr -> mod_name_list[i], '\r')) != NULL)
        if( pos < min_pos )
          min_pos = pos;
      if( min_pos != NULL )
        memset( min_pos, '\0', 
            MAX_LINE - ( min_pos - pt_ptr -> mod_name_list[i]) );
    }

    fclose( fp );

    // get data_vec_cnt;
    fp =  open_or_die( pt_ptr -> test_data, "r" );
    pt_ptr -> data_vec_cnt = 0;
    while (fgets(buf, MAX_LINE, fp) != NULL)
    {
      pt_ptr -> data_vec_cnt++;
    }
    rewind(fp);
    pt_ptr -> data_vec_list = (char**)malloc(
        sizeof( char* ) * pt_ptr -> data_vec_cnt );
    for( int i = 0; i < pt_ptr -> data_vec_cnt; ++i ){
      pt_ptr -> data_vec_list[i] = (char*)malloc(
          sizeof(char) * MAX_LINE );
    }

    for( int i = 0; i < pt_ptr->data_vec_cnt; ++i ){

      fgets(pt_ptr->data_vec_list[i], MAX_LINE, fp) ;

      // sanitize the change line symbols
      char *pos, *min_pos;
      min_pos = NULL;
      if ((pos=strchr(pt_ptr -> data_vec_list[i], '\n')) != NULL)
        min_pos = pos;
      if ((pos=strchr(pt_ptr -> data_vec_list[i], '\r')) != NULL)
        if( pos < min_pos )
          min_pos = pos;
      if( min_pos != NULL )
        memset( min_pos, '\0', 
            MAX_LINE - ( min_pos - pt_ptr -> data_vec_list[i]) );
    }
    fclose( fp );

  }

}

static void load_params( 
    void* ptr, // pointer of type Parameter_train or Parameter_test
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
    Parameter_test* pt_ptr = (Parameter_test*)ptr;
    free( pt_ptr -> model_list );
    free( pt_ptr -> test_data );
    free( pt_ptr -> results );

    for( int i = 0; i < pt_ptr -> model_cnt; ++ i )
      free( pt_ptr -> mod_name_list[i] );
    for( int i = 0; i < pt_ptr -> data_vec_cnt; ++ i )
      free( pt_ptr -> data_vec_list[i] );
    free( pt_ptr -> mod_name_list );
    free( pt_ptr -> data_vec_list );

    pt_ptr -> model_list    = NULL;
    pt_ptr -> test_data     = NULL;
    pt_ptr -> results       = NULL;
    pt_ptr -> mod_name_list = NULL;
    pt_ptr -> data_vec_list = NULL;
  }else if( type == GREEK_LETTERS ) {
    Data_wrapper*  dw_ptr   = (Data_wrapper*)(ptr);
    Greek_letters* gr_ptr   = dw_ptr -> gr_ptr;
    int            stt_cnt  = dw_ptr -> hmm_ptr -> state_num;
    int            emt_cnt  = dw_ptr -> hmm_ptr -> observ_num;

    for( int i = 0; i < stt_cnt; ++i ){
      free( gr_ptr -> alpha  [i] );
      free( gr_ptr -> beta   [i] );
      free( gr_ptr -> gamma  [i] );
    }
    free( gr_ptr -> alpha );
    free( gr_ptr -> beta  );
    free( gr_ptr -> gamma );
    free( gr_ptr -> gamma_end );
    gr_ptr -> gamma_end   = NULL;
    gr_ptr -> alpha       = NULL;
    gr_ptr -> beta        = NULL;
    gr_ptr -> gamma       = NULL;


    for( int i = 0; i < stt_cnt; ++i ){
      for( int j = 0; j < stt_cnt; ++j ){
        free( gr_ptr -> epsilon[i][j] );
      }
      free( gr_ptr -> epsilon[i] );
    }
    free( gr_ptr -> epsilon );
    gr_ptr -> epsilon     = NULL;

    for( int i = 0; i < emt_cnt; ++i ){
      for( int j = 0; j < stt_cnt; ++j ){
        free( gr_ptr -> gamma_arr[i][j] );
      }
      free( gr_ptr -> gam_end_arr[i] );
      free( gr_ptr -> gamma_arr  [i] );
    }
    gr_ptr -> gam_end_arr = NULL;
    gr_ptr -> gamma_arr   = NULL;

  }else {}
}

#endif // UTILS_HEADER_
