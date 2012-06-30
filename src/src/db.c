#include "frgg.h"


/**
 * @param dbp          DB structure handle returned by db_create()
 **/
int
dbopen(DB *dbp, const char *db_name, int create)
{
	u_int32_t flags = 0;   /* database open flags */


	if (create) {
		/* Database open flags */
		flags = DB_CREATE;    /* If the database does not exist, 
				       * create it.*/
		flags |= DB_TRUNCATE;
	}

	/* open the database */
	return dbp->open(dbp,        /* DB structure pointer */
			 NULL,       /* Transaction pointer */
			 db_name,    /* On-disk file that holds the database. */
			 NULL,       /* Optional logical database name */
			 DB_BTREE,   /* Database access method */
			 flags,      /* Open flags */
			 0);         /* File mode (using defaults) */
}


