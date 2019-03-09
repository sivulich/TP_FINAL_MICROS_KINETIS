/*
 * MP3PlayerData.h
 *
 *  Created on: Mar 8, 2019
 *      Author: Santi
 */

#ifndef MP3PLAYERDATA_H_
#define MP3PLAYERDATA_H_

#define MAX_VOLUME 	30
#define EQ_BANDS	3

typedef struct MP3PlayerData_ MP3PlayerData_;

extern MP3PlayerData_ MP3PlayerData;

struct MP3PlayerData_{
	int play;
	int offset;
	int volume;
	int currentScreen;
	int playMode;
	int equalizeBands[EQ_BANDS];
};

#endif /* MP3PLAYERDATA_H_ */
