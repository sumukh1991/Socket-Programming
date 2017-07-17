/*
* Copyright (c) 2016-2017 Wuklab, Purdue University. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/parser.h>
#include <linux/string.h>
#include <linux/bitmap.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
#include <linux/slab.h>
#include <linux/module.h>

#include "lego.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wuklab@Purdue");

/* Log file and device console */
const char *fname = "/root/lego-msg.log";
const char *confname = "/dev/console";

struct file *fp;
static DEFINE_MUTEX(fp_lock);

static void open_status_log_file(void)
{
	mm_segment_t old_fs;

	old_fs = get_fs(); // shouldn't this be set back? i.e., set_fs(old_fs) ? 
	set_fs(get_ds());

	fp = filp_open(fname, O_RDWR | O_CREAT | O_APPEND, 0644);
	if (IS_ERR_OR_NULL(fp))
		pr_err("Can not open: %s\n", fname);
}

static void close_status_log_file(void)
{
	if (!IS_ERR_OR_NULL(fp))
		filp_close(fp, NULL);
}

void log_status(const char *buf)
{
	if (!IS_ERR_OR_NULL(fp)) {
		mutex_lock(&fp_lock);
		kernel_write(fp, buf, strlen(buf), fp->f_pos); 
		mutex_unlock(&fp_lock);
	}   
}

struct file *confp;
static DEFINE_MUTEX(confp_lock);

/* Not printing to the console */
static ssize_t write_to_console(const char *buf, int count)
{
	mm_segment_t old_fs;
	ssize_t result;
	old_fs = get_fs(); 
	set_fs(get_ds()); // or can say KERNEL_DS/USER_DS specifically

	confp = filp_open(confname, O_WRONLY, 0);
	if (IS_ERR_OR_NULL(confp)) {
		pr_err("Can not open the console: %s\n", confname);
		return -EFAULT;
	}
	
	printk(KERN_INFO "Writing to console.\n");
	
	confp->f_pos = 0;

	if (!confp->f_op->write) {
		printk(KERN_INFO "Err writing to console.\n");
		mutex_lock(&confp_lock);
			kernel_write(confp, buf, strlen(buf), confp->f_pos); 
		mutex_unlock(&confp_lock);
		filp_close(confp, NULL);
		return -EINVAL;
	}

	mutex_lock(&confp_lock);
	result = confp->f_op->write(confp, buf, count, &confp->f_pos);
	mutex_unlock(&confp_lock);

	filp_close(confp, NULL);

	set_fs(old_fs);

	return result;
}

static char msg_buffer[256];

void log_msg_bytes(size_t nr_bytes)
{
	struct timespec ts; 
	int count;

	getnstimeofday(&ts);
	memset(msg_buffer, 0, 256);
	count = sprintf(msg_buffer, "%lu, %zu : Written through the proc/lego vfs.\n", ts.tv_sec, nr_bytes);
	/* Log the data in the console */
	log_status(msg_buffer);
	/* Print the output in the console */
	write_to_console(msg_buffer, count);
}

enum {
	OP_NODE, // manager node id
	OP_GPID, // global unique process id
	OP_STATUS // status information
/*
* OP_START_NODE, // start a node
* OP_STOP_NODE, // stop a node
* OP_START_PROC, // start a process
* OP_STOP_PROC, // terminate a process
* OP_FILE // file name to start
*/
};

static const match_table_t tokens = {
	{ OP_NODE, "node=%u"},
	{ OP_GPID, "gpid=%u"},
	{ OP_STATUS, "status"}
};

#define MAX_OPT_ARGS	4

static ssize_t lego_proc_read(struct file *file, char __user *buf,
				size_t count, loff_t *offs)
{
	// Read the information from the reponse to the userspace address
	return 0;
}

/*
* echo node=1,status > /proc/lego
* 	Query the status of the manager with node id 1
*
* echo gpid=10,status > /proc/lego
*	Query the status of the process with gpid 10
*/

static ssize_t lego_proc_write(struct file *file, const char __user *buf,
				size_t count, loff_t *offs)
{
#define MAX_NR_BYTES	64
	int option;
	char *p;
	char *options;
	substring_t args[MAX_OPT_ARGS];

	unsigned long node = 0, gpid = 0;

	bool status = false;

	printk(KERN_INFO "lego_proc_write: Started.\n");
	
	if (count > MAX_NR_BYTES)
		return -EINVAL;

	options = kzalloc(count, GFP_KERNEL);
	if (!options)
		return -ENOMEM;

	if (copy_from_user(options, buf, count)) {
		kfree(options);
		return -EFAULT;
	}

	printk(KERN_INFO "lego_proc_write: Parsing...\n");

	if (options[count - 1] == '\n')
		options[count - 1] = '\0';

	/* Parse the input to obtain the tokens */
	while ((p = strsep(&options, ",")) != NULL) {
		int token;
		if (!*p)
			continue;

		token = match_token(p, tokens, args);
		switch (token) {
			case OP_NODE:
				if (match_int(&args[0], &option))
					goto bad;
				node = (unsigned long)option;
				break;
			case OP_GPID:
				if (match_int(&args[0], &option))
					goto bad;
				gpid = (unsigned long)option;
				break;
			case OP_STATUS:
				status = true;
				break;
			default:
				pr_err("unknown option: '%s'", p);
				goto free;
		}
	}

	if (status) {
		if (node > 0) {
			// send request to node for the status information
			// write the response to the file and to the output
			
			printk(KERN_INFO "lego_proc_write: Node %lu status requested.\n", node);
		}
		if (gpid > 0) {
			// get the node id of the corresponding gpid from the global table 
			// send_request_to_node
			// write the response back to the log file
			printk(KERN_INFO "lego_proc_write: gpid %lu status requested.\n", gpid);
		}
	}
	
	log_msg_bytes((size_t)10);

	kfree(options);
	return count;

bad:
	pr_err("bad value '%s' for '%s'", args[0].from, p);
free:
	kfree(options);
	return -EINVAL;

#undef MAX_NR_BYTES
}

static const struct file_operations lego_proc_fops = {
	.owner = THIS_MODULE,
	.read	= lego_proc_read,
	.write	= lego_proc_write
};

/* 
* File in the proc fs where the query to obtain the status from the manager 
* is written to and read from. 
*/
int create_lego_proc_file(void)
{
	proc_create("lego", 0644, NULL, &lego_proc_fops);
	printk(KERN_INFO "called create_lego_proc_file\n");
	open_status_log_file();
	return 0;
}

void remove_lego_proc_file(void)
{
	remove_proc_entry("lego", NULL);
	close_status_log_file();
	printk(KERN_INFO "lego module removed.\n");
}

static void lego_module_init(void)
{
	printk(KERN_INFO "lego module init called.\n");
	create_lego_proc_file();
}

static void lego_module_exit(void)
{
	remove_lego_proc_file();
}

module_init(lego_module_init);
module_exit(lego_module_exit);
