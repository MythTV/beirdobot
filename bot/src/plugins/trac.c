/*
 *  This file is part of the beirdobot package
 *  Copyright (C) 2007 Gavin Hurlbut
 *
 *  This plugin is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This plugin is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this plugin; if not, write to the 
 *    Free Software Foundation, Inc., 
 *    51 Franklin Street, Fifth Floor, 
 *    Boston, MA  02110-1301  USA
 */

/*HEADER---------------------------------------------------
* $Id$
*
* Copyright 2007 Gavin Hurlbut
* All rights reserved
*/

/* INCLUDE FILES */
#define _BSD_SOURCE
#include "environment.h"
#include "botnet.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "structs.h"
#include "protos.h"
#include "logging.h"
#include "balanced_btree.h"
#include <svn_client.h>
#include <svn_auth.h>
#include <svn_pools.h>
#include <svn_config.h>
#include <svn_cmdline.h>
#include "mrss.h"
#include <curl/curl.h>

/* INTERNAL FUNCTION PROTOTYPES */
void regexpFuncTicket( IRCServer_t *server, IRCChannel_t *channel, char *who, 
                       char *msg, IRCMsgType_t type, int *ovector, int ovecsize,
                       void *tag );
void regexpFuncChangeset( IRCServer_t *server, IRCChannel_t *channel, 
                          char *who, char *msg, IRCMsgType_t type, 
                          int *ovector, int ovecsize, void *tag );
void botCmdTrac( IRCServer_t *server, IRCChannel_t *channel, char *who, 
                 char *msg, void *tag );
char *botHelpTrac( void *tag );
static int db_upgrade_schema( int current, int goal );
static void db_load_channel_regexp( void );
static void result_load_channel_regexp( MYSQL_RES *res, MYSQL_BIND *input, 
                                        void *args );
void *trac_thread(void *arg);
char *tracRecurseBuildChannelRegexp( BalancedBTreeItem_t *node );
void log_svn_error( svn_error_t *error );
static svn_error_t *simple_prompt_callback( svn_auth_cred_simple_t **cred,
                                            void *baton, const char *realm,
                                            const char *username,
                                            svn_boolean_t may_save,
                                            apr_pool_t *pool );
static svn_error_t *user_prompt_callback( svn_auth_cred_username_t **cred,
                                          void *baton, const char *realm,
                                          svn_boolean_t may_save,
                                          apr_pool_t *pool );
static svn_error_t *my_svn_receiver( void *baton, apr_hash_t *changed_paths, 
                                     svn_revnum_t revision, const char *author, 
                                     const char *date, const char *message, 
                                     apr_pool_t *pool );
static size_t tracMemorizeFile(void *ptr, size_t size, size_t nmemb, 
                               void *data);
void tracTicketCsv( BalancedBTree_t *tree, char *page );

/* CVS generated ID string */
static char ident[] _UNUSED_ = 
    "$Id$";

#define CURRENT_SCHEMA_TRAC 2
#define MAX_SCHEMA_QUERY 100

typedef QueryTable_t SchemaUpgrade_t[MAX_SCHEMA_QUERY];

static QueryTable_t defSchema[] = {
  { "CREATE TABLE `plugin_trac` (\n"
    "  `serverid` int(11) NOT NULL default '0',\n"
    "  `chanid` int(11) NOT NULL default '0',\n"
    "  `url` varchar(255) NOT NULL default '',\n"
    "  `svnUrl` varchar(255) NOT NULL default '',\n"
    "  `svnUser` varchar(64) NOT NULL default '',\n"
    "  `svnPasswd` varchar(64) NOT NULL default '',\n"
    "  PRIMARY KEY  (`serverid`, `chanid`)\n"
    ") TYPE = MyISAM\n", NULL, NULL, FALSE }
};
static int defSchemaCount = NELEMENTS(defSchema);

static SchemaUpgrade_t schemaUpgrade[CURRENT_SCHEMA_TRAC] = {
    /* 0 -> 1 */
    { { NULL, NULL, NULL, FALSE } },
    /* 1 -> 2 */
    { { "ALTER TABLE `plugin_trac` ADD `svnUrl` VARCHAR( 255 ) NOT NULL "
        "AFTER `url`, ADD `svnUser` VARCHAR( 64 ) NOT NULL AFTER `svnUrl`, "
        "ADD `svnPasswd` VARCHAR( 64 ) NOT NULL AFTER `svnUser`", NULL, NULL, 
        FALSE },
      { NULL, NULL, NULL, FALSE } }
};

static QueryTable_t tracQueryTable[] = {
    /* 0 */
    { "SELECT serverid, chanid, url, svnUrl, svnUser, svnPasswd "
      "FROM `plugin_trac` ORDER BY `chanid` ASC", NULL, NULL, FALSE }
};

typedef struct {
    int                 serverId;
    int                 chanId;
    IRCServer_t        *server;
    IRCChannel_t       *channel;
    char               *url;
    char               *svnUrl;
    char               *svnUser;
    char               *svnPasswd;
    apr_pool_t         *svnPool;
    svn_client_ctx_t   *svnContext;
} TracURL_t;

typedef struct {
    TracURL_t      *item;
    apr_pool_t     *pool;
    char           *message;
} TracSVNLog_t;

typedef struct {
    char               *title;
    char               *link;
    char               *desc;
    int                 commentCount;
    BalancedBTree_t    *csvTree;
} TracTicket_t;

typedef struct {
    char               *header;
    char               *data;
} TracCSV_t;

static char    *ticketRegexp = "(?i)(?:\\s|^)\\#(\\d+)(?:\\s|$)";
static char    *changesetRegexp = "(?i)(?:\\s|^)\\[(\\d+)\\](?:\\s|$)";
static char    *channelRegexp = NULL;

pthread_t           tracThreadId;
static bool         threadAbort = FALSE;
BalancedBTree_t    *urlTree;

int my_svn_initialize( TracURL_t *tracItem );
char *tracDetailsTicket( TracURL_t *tracItem, int number );
char *tracDetailsChangeset( TracURL_t *tracItem, int number );

void plugin_initialize( char *args )
{
    char                           *verString;
    int                             ver;
    int                             printed;

    LogPrintNoArg( LOG_NOTICE, "Initializing trac..." );

    ver = -1;
    printed = FALSE;
    do {
        verString = db_get_setting("dbSchemaTrac");
        if( !verString ) {
            ver = 0;
        } else {
            ver = atoi( verString );
            free( verString );
        }

        if( !printed ) {
            LogPrint( LOG_CRIT, "Current Trac database schema version %d", 
                                ver );
            LogPrint( LOG_CRIT, "Code supports version %d", 
                                CURRENT_SCHEMA_TRAC );
            printed = TRUE;
        }

        if( ver < CURRENT_SCHEMA_TRAC ) {
            ver = db_upgrade_schema( ver, CURRENT_SCHEMA_TRAC );
        }
    } while( ver < CURRENT_SCHEMA_TRAC );

    db_load_channel_regexp();

    thread_create( &tracThreadId, trac_thread, NULL, "thread_trac" );
}

void plugin_shutdown( void )
{
    BalancedBTreeItem_t    *item;
    TracURL_t              *tracItem;


    LogPrintNoArg( LOG_NOTICE, "Removing trac..." );
    if( channelRegexp ) {
        botCmd_remove( "trac" );
        regexp_remove( channelRegexp, ticketRegexp );
        regexp_remove( channelRegexp, changesetRegexp );
        free( channelRegexp );
    }

    threadAbort = TRUE;

    if( urlTree ) {
        BalancedBTreeLock( urlTree );
        while( urlTree->root ) {
            item = urlTree->root;
            tracItem = (TracURL_t *)item->item;
            free( tracItem->url );
            free( tracItem->svnUrl );
            free( tracItem->svnUser );
            free( tracItem->svnPasswd );
            if( tracItem->svnPool ) {
                svn_pool_destroy( tracItem->svnPool );
            }
            free( tracItem );
            BalancedBTreeRemove( urlTree, item, LOCKED, FALSE );
            free( item );
        }
        BalancedBTreeDestroy( urlTree );
    }
}

void *trac_thread(void *arg)
{
    char                   *string;
    int                     len;
    static char            *command = "trac";

    while( !threadAbort ) {
        if( !ChannelsLoaded ) {
            sleep( 5 );
            continue;
        }
        
        threadAbort = TRUE;

        BalancedBTreeLock( urlTree );
        if( !urlTree->root ) {
            LogPrintNoArg( LOG_INFO, "No Channels defined for Trac, "
                                     "disabling" );
            BalancedBTreeUnlock( urlTree );
            continue;
        }

        len = 7;
        channelRegexp = (char *)malloc(len);
        *channelRegexp = '\0';
        strcat(channelRegexp, "(?i)(" );

        string = tracRecurseBuildChannelRegexp( urlTree->root );
        BalancedBTreeUnlock( urlTree );

        len += strlen(string);
        channelRegexp = (char *)realloc(channelRegexp, len);
        strcat( channelRegexp, string );
        free( string );

        strcat( channelRegexp, ")" );

        LogPrint( LOG_INFO, "Trac: ChanRegexp %s", channelRegexp );

        regexp_add( (const char *)channelRegexp, (const char *)ticketRegexp, 
                    regexpFuncTicket, NULL );
        regexp_add( (const char *)channelRegexp, (const char *)changesetRegexp,
                    regexpFuncChangeset, NULL );
        botCmd_add( (const char **)&command, botCmdTrac, botHelpTrac, NULL );
    }

    return( NULL );
}


void regexpFuncTicket( IRCServer_t *server, IRCChannel_t *channel, char *who, 
                       char *msg, IRCMsgType_t type, int *ovector, int ovecsize,
                       void *tag )
{
    char                   *string;
    char                   *message;
    TracURL_t              *tracItem;
    BalancedBTreeItem_t    *item;

    item = BalancedBTreeFind( urlTree, &channel->channelId, UNLOCKED );
    if( !item ) {
        return;
    }

    tracItem = (TracURL_t *)item->item;

    string = regexp_substring( msg, ovector, ovecsize, 1 );
    if( string ) {
        message = (char *)malloc(28 + (2 * strlen(string)) + 
                                 strlen(tracItem->url) );
        sprintf( message, "Trac: Ticket #%s URL: %s/ticket/%s", string, 
                          tracItem->url, string );
        LogPrint( LOG_DEBUG, "%s in %s", message, channel->fullspec );
        LoggedChannelMessage( server, channel, message );
        free( message );
        free( string );
    }
}

void regexpFuncChangeset( IRCServer_t *server, IRCChannel_t *channel, 
                          char *who, char *msg, IRCMsgType_t type, 
                          int *ovector, int ovecsize, void *tag )
{
    char       *string;
    char       *message;
    TracURL_t              *tracItem;
    BalancedBTreeItem_t    *item;

    item = BalancedBTreeFind( urlTree, &channel->channelId, UNLOCKED );
    if( !item ) {
        return;
    }

    tracItem = (TracURL_t *)item->item;


    string = regexp_substring( msg, ovector, ovecsize, 1 );
    if( string ) {
        message = (char *)malloc(36 + (2 * strlen(string)) + 
                                 strlen(tracItem->url) );
        sprintf( message, "Trac: Changeset [%s] URL: %s/changeset/%s", string, 
                          tracItem->url, string );
        LogPrint( LOG_DEBUG, "%s in %s", message, channel->fullspec );
        LoggedChannelMessage( server, channel, message );
        free( message );
        free( string );
    }
}

static int db_upgrade_schema( int current, int goal )
{
    int                 i;

    if( current >= goal ) {
        return( current );
    }

    if( current == 0 ) {
        /* There is no dbSchema, assume that it is an empty database, populate
         * with the default schema
         */
        LogPrint( LOG_ERR, "Initializing Trac database to schema version %d",
                  CURRENT_SCHEMA_TRAC );
        for( i = 0; i < defSchemaCount; i++ ) {
            db_queue_query( i, defSchema, NULL, 0, NULL, NULL, NULL );
        }
        db_set_setting("dbSchemaTrac", "%d", CURRENT_SCHEMA_TRAC);
        return( CURRENT_SCHEMA_TRAC );
    }

    LogPrint( LOG_ERR, "Upgrading Trac database from schema version %d to "
                       "%d", current, current+1 );
    for( i = 0; schemaUpgrade[current][i].queryPattern; i++ ) {
        db_queue_query( i, schemaUpgrade[current], NULL, 0, NULL, NULL, NULL );
    }

    current++;

    db_set_setting("dbSchemaTrac", "%d", current);
    return( current );
}

static void db_load_channel_regexp( void )
{
    pthread_mutex_t        *mutex;

    urlTree = BalancedBTreeCreate( BTREE_KEY_INT );

    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init( mutex, NULL );

    db_queue_query( 0, tracQueryTable, NULL, 0, result_load_channel_regexp,
                    NULL, mutex );
    pthread_mutex_unlock( mutex );
    pthread_mutex_destroy( mutex );
    free( mutex );
}

static void result_load_channel_regexp( MYSQL_RES *res, MYSQL_BIND *input, 
                                        void *args )
{
    int                     count;
    int                     i;
    MYSQL_ROW               row;
    TracURL_t              *tracItem;
    BalancedBTreeItem_t    *item;

    if( !res || !(count = mysql_num_rows(res)) ) {
        channelRegexp = NULL;
        return;
    }

    BalancedBTreeLock( urlTree );

    for( i = 0; i < count; i++ ) {
        row = mysql_fetch_row(res);

        tracItem = (TracURL_t *)malloc(sizeof(TracURL_t));
        item     = (BalancedBTreeItem_t *)malloc(sizeof(BalancedBTreeItem_t));

        memset( tracItem, 0x00, sizeof(TracURL_t) );

        tracItem->serverId  = atoi(row[0]);
        tracItem->chanId    = atoi(row[1]);
        tracItem->url       = strdup(row[2]);
        tracItem->svnUrl    = strdup(row[3]);
        tracItem->svnUser   = strdup(row[3]);
        tracItem->svnPasswd = strdup(row[3]);

        if( *tracItem->svnUrl && 
            my_svn_initialize( tracItem ) != EXIT_SUCCESS ) {
            LogPrintNoArg( LOG_CRIT, "Trac: SVN Initialization error!" );
            if( tracItem->svnPool ) {
                svn_pool_destroy( tracItem->svnPool );
                tracItem->svnPool = NULL;
            }
        }

        item->item = (void *)tracItem;
        item->key  = (void *)&tracItem->chanId;
        BalancedBTreeAdd( urlTree, item, LOCKED, FALSE );
    }

    BalancedBTreeAdd( urlTree, NULL, LOCKED, TRUE );
    BalancedBTreeUnlock( urlTree );
}

char *tracRecurseBuildChannelRegexp( BalancedBTreeItem_t *node )
{
    char                   *message;
    char                   *oldmsg;
    char                   *submsg;
    int                     len;
    TracURL_t              *tracItem;

    message = NULL;

    if( !node ) {
        return( message );
    }

    submsg = tracRecurseBuildChannelRegexp( node->left );
    message = submsg;
    oldmsg  = message;
    if( message ) {
        len = strlen(message);
    } else {
        len = -1;
    }
    
    tracItem = (TracURL_t *)node->item;

    tracItem->server  = FindServerNum( tracItem->serverId );
    tracItem->channel = FindChannelNum( tracItem->server, tracItem->chanId );

    submsg = tracItem->channel->fullspec;
    message = (char *)realloc(message, len + 1 + strlen(submsg) + 2);
    if( oldmsg ) {
        strcat( message, "|" );
    } else {
        message[0] = '\0';
    }
    strcat( message, submsg );

    submsg = tracRecurseBuildChannelRegexp( node->right );
    if( submsg ) {
        len = strlen( message );

        message = (char *)realloc(message, len + 1 + strlen(submsg) + 2);
        strcat( message, "|" );
        strcat( message, submsg );
        free( submsg );
    }

    return( message );
}

void log_svn_error( svn_error_t *error )
{
    svn_error_t    *itr;
    char            buffer[256];

    for( itr = error; itr; itr = itr->child ) {
        *buffer = '\0';
        LogPrint( LOG_INFO, "SVN error (%d) %s: %s", itr->apr_err,
                            svn_strerror( itr->apr_err, buffer, 
                                          sizeof(buffer) ), itr->message );
    }

    svn_error_clear( error );
}

static svn_error_t *simple_prompt_callback( svn_auth_cred_simple_t **cred,
                                            void *baton, const char *realm,
                                            const char *username,
                                            svn_boolean_t may_save,
                                            apr_pool_t *pool )
{
    svn_auth_cred_simple_t *ret = apr_pcalloc( pool, sizeof(*ret) );
    TracURL_t *tracItem;

    tracItem = (TracURL_t *)baton;

    if( username ) {
        ret->username = apr_pstrdup( pool, username );
    } else {
        ret->username = apr_pstrdup( pool, tracItem->svnUser );
    }

    ret->password = apr_pstrdup( pool, tracItem->svnPasswd );

    *cred = ret;
    return( SVN_NO_ERROR );
}

static svn_error_t *user_prompt_callback( svn_auth_cred_username_t **cred,
                                          void *baton, const char *realm,
                                          svn_boolean_t may_save,
                                          apr_pool_t *pool )
{
    svn_auth_cred_username_t *ret = apr_pcalloc( pool, sizeof(*ret) );
    TracURL_t *tracItem;

    tracItem = (TracURL_t *)baton;

    ret->username = apr_pstrdup( pool, tracItem->svnUser );

    *cred = ret;
    return( SVN_NO_ERROR );
}

int my_svn_initialize( TracURL_t *tracItem ) 
{
    int                             retval;
    apr_pool_t                     *pool;
    svn_client_ctx_t               *ctx;
    svn_error_t                    *error;
    apr_array_header_t             *providers;
    svn_auth_provider_object_t     *provider;

    if( (retval = svn_cmdline_init( "plugin_trac", NULL ) ) != EXIT_SUCCESS ) {
        return( retval );
    }

    tracItem->svnPool = svn_pool_create(NULL);
    pool = tracItem->svnPool;

    if( (error = svn_config_ensure( NULL, pool )) ) {
        log_svn_error( error );
        return( EXIT_FAILURE );
    }
    
    if( (error = svn_client_create_context( &tracItem->svnContext, pool )) ) {
        log_svn_error( error );
        return( EXIT_FAILURE );
    }

    ctx = tracItem->svnContext;
        
    if( (error = svn_config_get_config( &ctx->config, NULL, pool )) ) {
        log_svn_error( error );
        return( EXIT_FAILURE );
    }

    providers = apr_array_make( pool, 2, sizeof(svn_auth_provider_object_t *));

#if ( SVN_VER_MAJOR == 1 && SVN_VER_MINOR == 3 )
    svn_client_get_simple_prompt_provider( &provider, simple_prompt_callback,
                                           (void *)tracItem, 2, pool );
#else
    svn_auth_get_simple_prompt_provider( &provider, simple_prompt_callback,
                                         (void *)tracItem, 2, pool );
#endif
    APR_ARRAY_PUSH( providers, svn_auth_provider_object_t *) = provider;

#if ( SVN_VER_MAJOR == 1 && SVN_VER_MINOR == 3 )
    svn_client_get_username_prompt_provider( &provider, 
                                             user_prompt_callback,
                                            (void *)tracItem, 2, pool );
#else
    svn_auth_get_username_prompt_provider( &provider, 
                                           user_prompt_callback,
                                          (void *)tracItem, 2, pool );
#endif
    APR_ARRAY_PUSH( providers, svn_auth_provider_object_t *) = provider;

    svn_auth_open( &ctx->auth_baton, providers, pool );
    
    return( EXIT_SUCCESS );
}

void botCmdTrac( IRCServer_t *server, IRCChannel_t *channel, char *who, 
                 char *msg, void *tag )
{
    char                   *line;
    char                   *command;
    char                   *which;
    char                   *message;
    int                     number;
    BalancedBTreeItem_t    *item;
    TracURL_t              *tracItem;


    command = CommandLineParse( msg, &line );
    if( !command ) {
        return;
    }

    message = NULL;

    if( !strcmp( command, "details" ) ) {
        free( command );
        which = CommandLineParse( line, &line );
        if( !which ) {
            LogPrintNoArg( LOG_CRIT, "No ticket/changeset" );
            return;
        }

        if( channel ) {
            item = BalancedBTreeFind( urlTree, &channel->channelId, UNLOCKED );
            if( !item ) {
                LogPrintNoArg( LOG_CRIT, "Channel has no Trac setup" );
                free( which );
                return;
            }
        } else {
            IRCChannel_t       *chan;

            message = CommandLineParse( line, &line );
            if( !message ) {
                LogPrintNoArg( LOG_CRIT, "No channel indicated" );
                free( which );
                return;
            }
            chan = FindChannel( server, message );
            free( message );

            if( !chan ) {
                LogPrintNoArg( LOG_CRIT, "No such channel" );
                free( which );
                return;
            }

            item = BalancedBTreeFind( urlTree, &chan->channelId, UNLOCKED );
            if( !item ) {
                LogPrintNoArg( LOG_CRIT, "Channel has no Trac setup" );
                free( which );
                return;
            }
        }
        
        tracItem = (TracURL_t *)item->item;

        if( *which == '#' ) {
            /* This is a ticket */

            /* Strip off the # */
            line = which + 1;
            number = atoi(line);
            free( which );

            if( !number ) {
                message = strdup( "Can't fetch details for #0 or non-numeric" );
            } else {
                message = tracDetailsTicket( tracItem, number );
            }
        } else if( *which == '[' && which[strlen(which)-1] == ']' ) {
            /* This is a changeset */

            /* Strip off the [] */
            line = which + 1;
            line[strlen(line)-1] = '\0';
            number = atoi(line);
            free( which );

            if( !number ) {
                message = strdup( "Can't fetch logs for [0] or non-numeric" );
            } else if( !tracItem->svnPool ) {
                message = strdup( "SVN Repository not configured!" );
            } else {
                message = tracDetailsChangeset( tracItem, number );
            }
        } else {
            free( which );
        }
    } else {
        free( command );
    }

    if( message ) {
        LogPrint( LOG_INFO, "Trac: %s", message );
        if( channel ) {
            LoggedChannelMessage( server, channel, message );
        } else {
            transmitMsg( server, TX_PRIVMSG, who, message);
        }

        free( message );
    }
}

char *botHelpTrac( void *tag )
{
    static char *help = "Show details for Trac tickets or changesets.  Syntax: "
                        "(in channel) trac details #ticknum | trac details "
                        "[chgsetnum]  (in privmsg) trac details #ticknum "
                        "channel | trac details [chgsetnum] channel";

    return( help );
}

static svn_error_t *my_svn_receiver( void *baton, apr_hash_t *changed_paths, 
                                     svn_revnum_t revision, const char *author, 
                                     const char *date, const char *message, 
                                     apr_pool_t *pool )
{
    TracSVNLog_t       *args;
    char               *ch;

    args = (TracSVNLog_t *)baton;

    args->message = (char *)malloc( 42 + strlen(author) + strlen(date) +
                                    strlen(message) );
    sprintf( args->message, "[%ld] Author: %s  Date: %s  Message: %s",
                            (long int)revision, author, date, message );

    for( ch = args->message; *ch; ch++ ) {
        if( *ch == '\n' || *ch == '\r' ) {
            *ch = ' ';
        }
    }

    return( NULL );
}

char *tracDetailsTicket( TracURL_t *tracItem, int number )
{
    mrss_t                 *data;
    mrss_item_t            *rssItem;
    mrss_error_t            ret;
    char                   *url;
    char                   *message;
    TracTicket_t           *ticket;
    BalancedBTreeItem_t    *item;
    CURL                   *curl;
    char                   *page;
    TracCSV_t              *csv;
    static char            *items[] = { "Reporter", "Owner", "Type", "Status",
                                        "Priority", "Milestone", "Component",
                                        "Version", "Severity", "Resolution" };
    static int              itemCount = NELEMENTS(items);
    int                     i;
    int                     len;

    message = NULL;
    ticket = (TracTicket_t *)malloc(sizeof(TracTicket_t));
    memset( ticket, 0x00, sizeof(TracTicket_t) );

    url = (char *)malloc(strlen(tracItem->url) + 34);
    sprintf( url, "%s/ticket/%d?format=rss", tracItem->url, number );

    ret = mrss_parse_url( url, &data );
    free( url );

    if( ret ) {
        message = (char *)malloc(60);
        sprintf( message, "No data for ticket #%d", number );
        LogPrint( LOG_CRIT, "%s: Couldn't get RSS", message );
        return( message );
    }

    mrss_get( data, MRSS_FLAG_TITLE, &ticket->title, 
                    MRSS_FLAG_LINK, &ticket->link,
                    MRSS_FLAG_DESCRIPTION, &ticket->desc, 
                    MRSS_FLAG_END );

    ticket->commentCount = 0;
    for( rssItem = data->item; rssItem; rssItem = rssItem->next ) {
        ticket->commentCount++;
    }

    mrss_free( data );

    message = strdup(ticket->title);

    len = strlen( message ) + 24;
    message = (char *)realloc( message, len );
    sprintf( &(message[strlen(message)]), ", Comments: %d", 
             ticket->commentCount );
    len = strlen( message ) + 1;

    ticket->csvTree = BalancedBTreeCreate( BTREE_KEY_STRING );
    BalancedBTreeLock( ticket->csvTree );

    page = NULL;
    curl = curl_easy_init();
    if( curl ) {
        url = (char *)malloc(strlen(tracItem->url) + 34);
        sprintf( url, "%s/ticket/%d?format=csv", tracItem->url, number );

        curl_easy_setopt( curl, CURLOPT_URL, url);
        curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, tracMemorizeFile );
        curl_easy_setopt( curl, CURLOPT_FOLLOWLOCATION, 1 );
        curl_easy_setopt( curl, CURLOPT_FILE, (void *)&page );
        curl_easy_setopt( curl, CURLOPT_TIMEOUT, 10 );

        if( curl_easy_perform( curl ) ) {
            if( page ) {
                free( page );
                page = NULL;
            }
        }

        curl_easy_cleanup( curl );
    }

    if( page ) {
        tracTicketCsv( ticket->csvTree, page );
        free( page );
    }

    for( i = 0; i < itemCount; i++ ) {
        item = BalancedBTreeFind( ticket->csvTree, &items[i], LOCKED );
        if( item ) {
            csv = (TracCSV_t *)item->item;

            if( *csv->data ) {
                len += strlen( items[i] ) + strlen( csv->data ) + 4;
                message = (char *)realloc(message, len);
                sprintf( &(message[strlen(message)]), ", %s: %s", items[i],
                                                      csv->data );
            }
        }
    }

    while( ticket->csvTree->root ) {
        item = ticket->csvTree->root;
        csv = (TracCSV_t *)item->item;

        free( csv->header );
        free( csv->data );
        free( csv );

        BalancedBTreeRemove( ticket->csvTree, item, LOCKED, FALSE );
        free( item );
    }

    BalancedBTreeDestroy( ticket->csvTree );

    free( ticket->title );
    free( ticket->link );
    free( ticket->desc );
    free( ticket );

    return( message );
}

char *tracDetailsChangeset( TracURL_t *tracItem, int number )
{
    svn_error_t            *error;
    apr_pool_t             *pool;
    svn_opt_revision_t      revision;
    apr_array_header_t     *target;
    char                   *url;
    TracSVNLog_t            args;
    char                   *message;

    pool = svn_pool_create( tracItem->svnPool );
    revision.kind         = svn_opt_revision_number;
    revision.value.number = number;

    target = apr_array_make( pool, 1, sizeof(const char *));
    url = apr_pstrdup( pool, tracItem->svnUrl );
    APR_ARRAY_PUSH( target, const char *) = url;

    args.item    = tracItem;
    args.pool    = pool;
    args.message = NULL;

    if( (error = svn_client_log( target, &revision, &revision, 
                                 FALSE, FALSE, my_svn_receiver, 
                                 (void *)&args, 
                                 tracItem->svnContext, pool ) ) ) {
        log_svn_error( error );
        message = (char *)malloc(60);
        sprintf(message, "No data for changeset [%d]", number);
    } else {
        message = args.message;
    }

    svn_pool_destroy( pool );
    return( message );
}

static size_t tracMemorizeFile(void *ptr, size_t size, size_t nmemb, 
                               void *data)
{
    int    realsize;
    char **page;
    char  *pageData;

    realsize = size * nmemb;
    page = (char **)data;

    pageData = (char *)malloc( realsize + 1 );
    memcpy(pageData, ptr, realsize);

    *page = pageData;
    return( realsize );
}

void tracTicketCsv( BalancedBTree_t *tree, char *page )
{
    char                   *headers;
    char                   *data;
    char                   *ch;
    TracCSV_t              *csv;
    BalancedBTreeItem_t    *item;
    int                     len;

    ch = strchr( (const char *)page, '\n' );
    *ch = '\0';
    headers = page;

    /* skip the \r */
    data = ch + 1;
    ch = strchr( (const char *)data, '\n' );
    *ch = '\0';

    for( ; *headers && *data; ) {
        csv = (TracCSV_t *)malloc(sizeof(TracCSV_t));
        ch = strchr( (const char *)headers, ',' );
        if( !ch ) {
            len = strlen( headers );
        } else {
            len = ch - headers;
        }
        csv->header = strndup( headers, len );
        headers += len + 1;

        ch = strchr( (const char *)data, ',' );
        if( !ch ) {
            len = strlen( data );
        } else {
            len = ch - data;
        }
        csv->data = strndup( data, len );
        data += len + 1;

        item = (BalancedBTreeItem_t *)malloc(sizeof(BalancedBTreeItem_t));
        item->item = (void *)csv;
        item->key  = (void *)&csv->header;

        BalancedBTreeAdd( tree, item, LOCKED, FALSE );
    }

    BalancedBTreeAdd( tree, NULL, LOCKED, TRUE );
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
