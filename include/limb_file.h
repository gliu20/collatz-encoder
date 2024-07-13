#pragma once

#include <stdio.h>
#include "limb_dlist.h"

/**
 * Reads or write a file from limb list to file.
 * We expect the caller to properly close and destroy
 * the appropriate handles.
 * 
 * when __SIZE_MAX__ is returned, it means a read or
 * write has failed. otherwise we return the number of
 * bytes read or written
 */
size_t read_file(limb_dlist_t* ll, FILE *file);
size_t write_file(limb_dlist_t* ll, FILE *file);
