/*
* Copyright (c) 2016-2017 Wuklab, Purdue University. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/lego.h>

static int lego_module_init(void)
{
	create_lego_proc_file();
}

static void lego_module_exit(void)
{
	remove_lego_proc_file();
}

module_init(lego_module_init);
module_exit(lego_module_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wuklab@Purdue");