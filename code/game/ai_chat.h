/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006-2010 Lars '0xA5EA' Kandler

This file is part of KingpinQ3 source code.

KingpinQ3 source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

KingpinQ3 source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//

/*****************************************************************************
 * name:		ai_chat.h
 *
 * desc:		Quake3 bot AI
 *
 * $Archive: /source/code/botai/ai_chat.c $
 *
 *****************************************************************************/
#ifndef AI_CHAT_H_
#define AI_CHAT_H_
//
int BotChat_EnterGame(bot_state_t *bs);
//
int BotChat_ExitGame(bot_state_t *bs);
//
int BotChat_StartLevel(bot_state_t *bs);
//
int BotChat_EndLevel(bot_state_t *bs);
//
int BotChat_HitTalking(bot_state_t *bs);
//
int BotChat_HitNoDeath(bot_state_t *bs);
//
int BotChat_HitNoKill(bot_state_t *bs);
//
int BotChat_Death(bot_state_t *bs);
//
int BotChat_Kill(bot_state_t *bs);
//
int BotChat_EnemySuicide(bot_state_t *bs);
//
int BotChat_Random(bot_state_t *bs);
// time the selected chat takes to type in
float BotChatTime(bot_state_t *bs);
// returns true if the bot can chat at the current position
int BotValidChatPosition(bot_state_t *bs);
// test the initial bot chats
void BotChatTest(bot_state_t *bs);

#endif // AI_CHAT_H_

