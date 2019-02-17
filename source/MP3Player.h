/*
 * MP3Player.h
 *
 *  Created on: Feb 16, 2019
 *      Author: santi
 */

#ifndef MP3PLAYER_H_
#define MP3PLAYER_H_

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct MP3Player_ MP3Player_;

extern MP3Player_ MP3Player;

struct MP3Player_{
	int (*init)();
	void (*update)(int play, int volume);

};

#if defined(__cplusplus)
}
#endif

#endif /* MP3PLAYER_H_ */
