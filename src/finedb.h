#ifndef __FINEDB_H__
#define __FINEDB_H__

#include <pthread.h>
#include "lmdb.h"
#include "ydefs.h"
#include "yvect.h"

/** @const DEFAULT_NBR_THREADS Default number of connection threads. */
#define DEFAULT_NBR_THREADS	15
/** @const DEFAULT_DB_PATH Default path to the database. */
#define DEFAULT_DB_PATH		"../var/database"
/** @const DEFAULT_PORT Default port number. */
#define DEFAULT_PORT		11138

/**
 * @typedef	Main structure of the FineDB application.
 * @field	run			YTRUE while the server must be running.
 * @field	database		LMDB environment.
 * @field	socket			Socket descriptor for incoming connections.
 * @field	threads_socket		Nanomsg socket for threads communication.
 * @field	writer_tid		ID of the writer thread.
 * @field	tcp_threads		List of connection threads.
 */
typedef struct finedb_s {
	ybool_t run;
	MDB_env	*database;
	int socket;
	int threads_socket;
	pthread_t writer_tid;
	yvect_t tcp_threads;
} finedb_t;

#endif /* __FINEDB_H__ */