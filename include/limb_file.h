#pragma once

#include <stdio.h>
#include "limb_dlist.h"

size_t read_file(limb_dlist_t* ll, FILE *file);
size_t write_file(limb_dlist_t* ll, FILE *file);