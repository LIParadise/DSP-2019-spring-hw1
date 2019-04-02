#include <string.h>

#ifndef CALC_HEADER_
#define CALC_HEADER_

#ifndef MAX_STATE
#	define MAX_STATE	10
#endif

#ifndef MAX_OBSERV
#	define MAX_OBSERV	26
#endif

#ifndef MAX_SEQ
#	define	MAX_SEQ		200
#endif

#ifndef MAX_LINE
#	define MAX_LINE 	256
#endif

typedef struct{
  double alpha         [MAX_STATE][MAX_LINE];
  double beta          [MAX_STATE][MAX_LINE];
  double gamma         [MAX_STATE][MAX_LINE];
  double epsilon       [MAX_STATE][MAX_LINE];
} Greek_letters;

typedef struct{
  Parameter_train* pr_ptr;
  Greek_letters*   gr_ptr;
  HMM*             hmm_ptr;
  int              cur_line_idx;  // current line index
} Data_wrapper;

static void init_greek ( Greek_letters* gr_ptr ){
  memset(gr_ptr -> alpha  ,'\0', MAX_STATE*MAX_LINE*sizeof(double));
  memset(gr_ptr -> beta   ,'\0', MAX_STATE*MAX_LINE*sizeof(double));
  memset(gr_ptr -> gamma  ,'\0', MAX_STATE*MAX_LINE*sizeof(double));
  memset(gr_ptr -> epsilon,'\0', MAX_STATE*MAX_LINE*sizeof(double));
}

static void fill_alpha ( Data_wrapper* dw_ptr){

  Parameter_train * pr_ptr   = dw_ptr -> pr_ptr;
  Greek_letters   * gr_ptr   = dw_ptr -> gr_ptr;
  HMM             * hmm_ptr  = dw_ptr -> hmm_ptr;
  int               line_cnt = dw_ptr -> cur_line_idx;

  for( int state_idx = 0; state_idx < MAX_STATE; ++state_idx ){
    gr_ptr -> alpha[state_idx][0] = 
      (hmm_ptr -> initial[state_idx]) * ( hmm_ptr -> 
          observation[pr_ptr -> model_data[i][0]-'A'][state_idx] );
  }

  for( int observ_idx = 1; 
      pr_ptr -> model_data[i][observ_idx] != '\0';
      ++observ_idx ){
    for( int state_idx = 0; state_idx < MAX_STATE; ++state_idx ){
      gr_ptr -> alpha[state_idx][observ_idx]
    }
  }

}


#endif // CALC_HEADER_
