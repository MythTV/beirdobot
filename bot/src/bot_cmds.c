/*
 *  This file is part of the beirdobot package
 *  Copyright (C) 2006 Gavin Hurlbut
 *
 *  beirdobot is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*HEADER---------------------------------------------------
* $Id$
*
* Copyright 2006 Gavin Hurlbut
* All rights reserved
*/

/* INCLUDE FILES */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "botnet.h"
#include "environment.h"
#include "structs.h"

/* INTERNAL FUNCTION PROTOTYPES */
void botCmdHelp( IRCServer_t *server, IRCChannel_t *channel, char *who, 
                 char *msg );
void botCmdSearch( IRCServer_t *server, IRCChannel_t *channel, char *who,
                   char *msg );
void botCmdSeen( IRCServer_t *server, IRCChannel_t *channel, char *who, 
                 char *msg );
void botCmdTrout( IRCServer_t *server, IRCChannel_t *channel, char *who, 
                  char *msg );

/* CVS generated ID string */
static char ident[] _UNUSED_ = 
    "$Id$";

static BotCmd_t botCmd[] _UNUSED_ = {
    { "help",       botCmdHelp },
    { "search",     botCmdSearch },
    { "seen",       botCmdSeen },
    { "trout",      botCmdTrout }
};
static int botCmdCount _UNUSED_ = NELEMENTS(botCmd);

BalancedBTree_t    *botCmdTree;


void botCmd_initialize( void )
{
    int                     i;
    BalancedBTreeItem_t    *item;

    botCmdTree = BalancedBTreeCreate( BTREE_KEY_STRING );
    BalancedBTreeLock( botCmdTree );

    for( i = 0; i < botCmdCount; i++ ) {
        item = (BalancedBTreeItem_t *)malloc(sizeof(BalancedBTreeItem_t));
        if( item ) {
            item->item = (void *)botCmd[i].func;
            item->key  = (void *)botCmd[i].command;
            BalancedBTreeAdd( botCmdTree, item, LOCKED, FALSE );
        }
    }

    BalancedBTreeAdd( botCmdTree, NULL, LOCKED, TRUE );
    BalancedBTreeUnlock( botCmdTree );
}

void botCmd_add( char *command, BotCmdFunc_t func )
{
    BalancedBTreeItem_t    *item;

    item = (BalancedBTreeItem_t *)malloc(sizeof(BalancedBTreeItem_t));
    if( !item ) {
        return;
    }

    item->item = (void *)func;
    item->key  = (void *)command;
    BalancedBTreeAdd( botCmdTree, item, UNLOCKED, TRUE );
}

void botCmd_parse( IRCServer_t *server, IRCChannel_t *channel, char *who, 
                   char *msg )
{
    char           *line;
    char           *cmd;
    int             len;
    void           *func;
    BotCmdFunc_t    cmdFunc;

    line = strstr( msg, " " );
    if( line ) {
        /* Command has trailing text, skip the space */
        len = line - msg;
        line++;

        cmd = (char *)malloc( len + 2 );
        strncpy( cmd, msg, len );
        cmd[len] = '\0';
    } else {
        /* Command is the whole line */
        cmd = strdup( msg );
    }

    func = BalancedBTreeFind( botCmdTree, (void *)cmd, UNLOCKED );
    if( func ) {
        cmdFunc = (BotCmdFunc_t)func;
        cmdFunc( server, channel, who, line );
    }
}


void botCmdHelp( IRCServer_t *server, IRCChannel_t *channel, char *who, 
                 char *msg )
{
    if( !msg ) {
        printf( "Bot CMD: Help NULL by %s\n", who );
    } else {
        printf( "Bot CMD: Help %s by %s\n", msg, who );
    }
}

void botCmdSearch( IRCServer_t *server, IRCChannel_t *channel, char *who, 
                   char *msg )
{
    if( !msg ) {
        printf( "Bot CMD: Search NULL by %s\n", who );
    } else {
        printf( "Bot CMD: Search %s by %s\n", msg, who );
    }
}

void botCmdSeen( IRCServer_t *server, IRCChannel_t *channel, char *who, 
                 char *msg )
{
    if( !msg ) {
        printf( "Bot CMD: Seen NULL by %s\n", who );
    } else {
        printf( "Bot CMD: Seen %s by %s\n", msg, who );
    }
}

void botCmdTrout( IRCServer_t *server, IRCChannel_t *channel, char *who, 
                  char *msg )
{
    static char message[MAX_STRING_LENGTH];

    if( !msg ) {
        printf( "Bot CMD: Trout NULL by %s\n", who );
    } else {
        printf( "Bot CMD: Trout %s by %s\n", msg, who );
    }

    if( !channel ) {
        BN_SendPrivateMessage(&server->ircInfo, (const char *)who,
                              "You need to do this in a public channel!");
        return;
    }

    if( !msg ) {
        sprintf( message, "dumps a bucket of trout onto %s", who );
    } else {
        sprintf( message, "slaps %s with a trout on behalf of %s...", msg, 
                 who );
    }
    BN_SendActionMessage( &server->ircInfo, (const char *)channel->channel,
                          (const char *)message );
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
