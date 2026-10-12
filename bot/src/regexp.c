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
#include "protos.h"
#include "linked_list.h"
#include "logging.h"

/* INTERNAL FUNCTION PROTOTYPES */

/* CVS generated ID string */
static char ident[] _UNUSED_ = 
    "$Id$";

static char *channelsAll = "^.*$";
LinkedList_t   *regexpList;

void regexp_initialize( void )
{
    static char pcreVersion[32];

    regexpList = LinkedListCreate();

    pcre2_config( PCRE2_CONFIG_VERSION, pcreVersion );
    versionAdd( "pcre", pcreVersion );
}

void regexp_add( const char *channelRegexp, const char *contentRegexp, 
                 RegexpFunc_t func, void *tag )
{
    Regexp_t    *item;
    int          errcode;
    PCRE2_SIZE   erroffset;
    PCRE2_UCHAR  error[256];

    item = (Regexp_t *)malloc(sizeof(Regexp_t));
    if( !item || !func ) {
        free( item );
        return;
    }

    if( !channelRegexp ) {
        channelRegexp = channelsAll;
    }

    item->tag = tag;

    item->channelRegexp = channelRegexp;
    item->contentRegexp = contentRegexp;

    item->reChannel = pcre2_compile( (PCRE2_SPTR)channelRegexp,
                                     PCRE2_ZERO_TERMINATED, 0, &errcode,
                                     &erroffset, NULL );
    if( !item->reChannel ) {
        pcre2_get_error_message( errcode, error, sizeof(error) );
        LogPrint( LOG_CRIT, "%s at offset %zu", (char *)error, erroffset );
        free( item );
        return;
    }
    pcre2_jit_compile( item->reChannel, PCRE2_JIT_COMPLETE );

    item->reContent = pcre2_compile( (PCRE2_SPTR)contentRegexp,
                                     PCRE2_ZERO_TERMINATED, 0, &errcode,
                                     &erroffset, NULL );
    if( !item->reContent ) {
        pcre2_get_error_message( errcode, error, sizeof(error) );
        LogPrint( LOG_CRIT, "%s at offset %zu", (char *)error, erroffset );
        pcre2_code_free( item->reChannel );
        free( item );
        return;
    }
    pcre2_jit_compile( item->reContent, PCRE2_JIT_COMPLETE );

    item->func      = func;

    LinkedListAdd( regexpList, (LinkedListItem_t *)item, UNLOCKED, AT_TAIL );
}

void regexp_remove( char *channelRegexp, char *contentRegexp )
{
    LinkedListItem_t   *item;
    Regexp_t           *regexp;

    if( !channelRegexp ) {
        channelRegexp = channelsAll;
    }

    LinkedListLock( regexpList );

    for( item = regexpList->head; item; item = item->next ) {
        regexp = (Regexp_t *)item;

        if( !strcmp( channelRegexp, regexp->channelRegexp ) &&
            !strcmp( contentRegexp, regexp->contentRegexp ) ) {
            LinkedListRemove( regexpList, item, LOCKED );

            pcre2_code_free( regexp->reChannel );
            pcre2_code_free( regexp->reContent );
            free( item );
            LinkedListUnlock( regexpList );
            return;
        }
    }

    LinkedListUnlock( regexpList );
}

void regexp_parse( IRCServer_t *server, IRCChannel_t *channel, char *who, 
                   char *msg, IRCMsgType_t type )
{
    LinkedListItem_t   *item;
    Regexp_t           *regexp;
    pcre2_match_data   *match;
    PCRE2_SIZE         *ov;
    int                 rc;
    int                 i;
    int                 pairs;
    PCRE2_SIZE          lenMsg;
    PCRE2_SIZE          lenChan;
    int                 ovector[30];
    PCRE2_SIZE          start;

    if( !channel ) {
        return;
    }

    /* 10 pairs: same capacity as the old int ovector[30] */
    match = pcre2_match_data_create( 10, NULL );
    if( !match ) {
        return;
    }
    ov = pcre2_get_ovector_pointer( match );

    lenMsg  = strlen( msg );
    lenChan = strlen( channel->fullspec );

    LinkedListLock( regexpList );

    for( item = regexpList->head; item; item = item->next ) {
        regexp = (Regexp_t *)item;

        rc = pcre2_match( regexp->reChannel, (PCRE2_SPTR)channel->fullspec,
                          lenChan, 0, 0, match, NULL );
        if( rc < 0 ) {
            /* Channels don't match */
            continue;
        }

        for( start = 0; ; start = ov[1] ) {
            rc = pcre2_match( regexp->reContent, (PCRE2_SPTR)msg, lenMsg,
                              start, 0, match, NULL );
            if( rc < 0 ) {
                break;
            }

            /* Hand plugins the same int ovector they always got */
            pairs = rc ? rc : 10;
            for( i = 0; i < 2 * pairs; i++ ) {
                ovector[i] = ( ov[i] == PCRE2_UNSET ) ? -1 : (int)ov[i];
            }

            /* We got a channel and content match, call the function */
            regexp->func( server, channel, who, msg, type, ovector, rc,
                          regexp->tag );

            /* rc == 0: too many groups (old code stopped too);
             * empty match at start: would loop forever */
            if( rc == 0 || ov[1] <= start ) {
                break;
            }
        }
    }

    LinkedListUnlock( regexpList );
    pcre2_match_data_free( match );
}

char *regexp_substring( char *msg, int *ovector, int stringcount,
                        int stringnum )
{
    char        *string;
    int          start;
    int          len;

    if( stringnum < 0 || stringnum >= stringcount ) {
        return( NULL );
    }

    /* Unset group returns "", as pcre_get_substring() did */
    start = ovector[2 * stringnum];
    len   = ( start < 0 ) ? 0 : ovector[2 * stringnum + 1] - start;

    string = (char *)malloc( len + 1 );
    if( !string ) {
        return( NULL );
    }
    if( len ) {
        memcpy( string, msg + start, len );
    }
    string[len] = '\0';

    return( string );
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
