/* lookup table.h                               */
/* this is predominately a chomp of minclookup, */
/* so we'll blame peter when it breaks          */

#ifndef LOOKUP_TABLE_H
#define LOOKUP_TABLE_H

#include "globals.h"

/* Function prototypes */
Lookup_Table *new_lookup_table(void);
Lookup_Table *read_lookup_table(char *lookup_file);
int      store_table(Lookup_Table * lut, Main_info * ptr);
int      load_table_to_pane(Pane_info pane);
void     init_lookup_tables(Main_info * ptr);
GList   *get_cmaps_list(Main_info * ptr);
Lookup_Table *get_cmap_ptr(char *cmap_name, Main_info * ptr);

#endif
