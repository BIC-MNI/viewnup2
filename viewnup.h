/* viewnup.h */

#ifndef VIEWNUP_H
#define VIEWNUP_H

#include "globals.h"

/* function prototypes */
Main_info *get_main_ptr(void);

View_info init_view_info(int type);
void     free_view_info(View_info view);

Pane_info init_pane_info(Synch_info * synch);
void     free_pane_info(Pane_info pane);
void     print_pane_info(Pane_info pane);
void     build_pane_filelist(Main_info * ptr);

void     init_rot_vec(View_info view);
void     init_tilt_vec(View_info view);
void     init_view_idx(View_info view);

Synch_info *init_synch_info(void);
int      remove_synch(Main_info * ptr, Synch_info * synch);
int      get_synch_idx(Main_info * ptr, Synch_info * synch);
Synch_info *get_next_synch(Main_info * ptr, Synch_info * synch);

#endif
