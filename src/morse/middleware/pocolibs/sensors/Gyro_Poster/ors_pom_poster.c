#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <pom/pomStruct.h>

#include "ors_pom_poster.h"

static char* ref_name;
static POSTER_ID ref_id;

POSTER_ID init_data (const char* poster_name, const char* reference_frame, 
					 float confidence, int* ok)
{
	POSTER_ID id;

	STATUS s = posterCreate (poster_name, sizeof(POM_ME_POS), &id);
	if (s == ERROR)
	{
		char buf[1024];
		h2getErrMsg(errnoGet(), buf, sizeof(buf));
		printf ("Unable to create the %s poster : %s\n", poster_name, buf);
        *ok = 0;
		return (NULL);
	}

	printf ("INIT ID = %p (pointer)\n", id);
	ref_name = strdup(reference_frame);

	POM_ME_POS* pos = posterAddr(id);
	memset(pos, 0, sizeof(POM_ME_POS));
	pos->kind = POM_ME_ABSOLUTE;
	pos->confidence = confidence;

    *ok = 1;

	return (id);
}

/*
 * Yaw, pitch, roll are in degree in input
 */
int post_data( POSTER_ID id, double x, double y, double z, 
						     double yaw, double pitch, double roll)
{
	// Variables to use for writing the poster
	int offset = 0;

	// try to get the pom reference frame
	// if we can't get it, just returns
	if (ref_id == NULL) {
		fprintf(stderr, "ref id is NULL : searching for %s\n", ref_name);
		if (posterFind(ref_name, &ref_id) == ERROR) {
			fprintf(stderr, "can't find %s : looping\n", ref_name);
			ref_id = NULL;
			return -1;
			}
	}

	// Declare local versions of the structures used
	POM_SENSOR_POS framePos;

	posterRead(ref_id, 0, &framePos, sizeof(POM_SENSOR_POS));

	POM_ME_POS* pos = posterAddr(id);
	posterTake(id, POSTER_WRITE);
	// Fill in the POM_POS_EULER
	// yaw, pitch, roll are expected in radian

#define DEG_TO_RAD(x) ((x)*M_PI/180.)

	pos->main.euler.yaw = DEG_TO_RAD(yaw);
	pos->main.euler.pitch = DEG_TO_RAD(pitch);
	pos->main.euler.roll = DEG_TO_RAD(roll);

#undef DEG_TO_RAD

	pos->main.euler.x = x;
	pos->main.euler.y = y;
	pos->main.euler.z = z;

	pos->date1 = framePos.date;
	posterGive(id);
	return 0;
}


int finalize (POSTER_ID id)
{
	posterDelete(id);
	free(ref_name);

	return 0;
}
