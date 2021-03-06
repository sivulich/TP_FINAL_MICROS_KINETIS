/*
 * MP3PlayerData.h
 *
 *  Created on: Mar 8, 2019
 *      Author: Santi
 */

#ifndef MP3PLAYERDATA_H_
#define MP3PLAYERDATA_H_

#define MAX_VOLUME 	30
#define MIN_VOLUME 	0
#define STARTING_VOLUME 15
#define EQ_BANDS	3
#define EQ_MAX_DB 6
#define EQ_STEP_DB 1
#define VUMETERS_MODES 6

typedef struct MP3PlayerData_ MP3PlayerData_;

extern MP3PlayerData_ MP3PlayerData;

struct MP3PlayerData_{
	int play;
	int offset;
	int volume;
	int currentScreen;
	int playMode;
	int vumeterMode;
	int equalizeBands[EQ_BANDS];
};

#endif /* MP3PLAYERDATA_H_ */
