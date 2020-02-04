#ifndef __MY_MODULE_H
#define __MY_MODULE_H

#define MAJOR_NUM 	511
#define MINOR_START 	0
#define MINOR_LAST 	3
#define MINOR_COUNT	4
#define DEV_NAME	"scull_char"

/*
 * Macros to help debugging
 */

#undef PDEBUG             /* undef it, just in case */
#ifdef SCULL_DEBUG
#  ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) pr_alert("scull: " fmt, ## args)
#  else
     /* This one for user space */
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#undef PDEBUGG
#define PDEBUGG(fmt, args...) /* nothing: it's a placeholder */


/*
 * The bare device is a variable-length region of memory.
 * Use a linked list of indirect blocks.
 *
 * "Scull_Dev->data" points to an array of pointers, each
 * pointer refers to a memory area of SCULL_QUANTUM bytes.
 *
 * The array (quantum-set) is SCULL_QSET long.
 */
#ifndef SCULL_QUANTUM
#define SCULL_QUANTUM 4000
#endif

#ifndef SCULL_QSET
#define SCULL_QSET    1000
#endif

struct scull_qset {
	void **data;
	struct scull_qset *next;
};

typedef struct scull_dev {
	void **data; 			
	struct scull_dev *next;		/* next listitem */
	int quantum; 			/* the current quantum size */
	int qset;			/* the current array size */
	unsigned long size;		/* amount of data stored here */
	unsigned int access_key; 	/* used by sculluid and scullpriv */
	struct semaphore sem;		/* mutual exclusion semaphore */
	struct cdev cdev; 		/* Char device structure */
}scull_dev;

#endif /* __MY_MODULE_H */
