/*
 *  This file is part of the beirdonet package
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
*
*/

#ifndef protos_h_
#define protos_h_

#include "linked_list.h"
#include "balanced_btree.h"
#include "structs.h"

/* CVS generated ID string (optional for h files) */
static char protos_h_ident[] _UNUSED_ = 
    "$Id$";

/* Externals */
extern char   *mysql_host;
extern uint16  mysql_portnum;
extern char   *mysql_user;
extern char   *mysql_password;
extern char   *mysql_db;
extern LinkedList_t   *ServerList;
extern bool verbose;
extern bool Debug;
extern bool Daemon;
extern bool GlobalAbort;
extern bool BotDone;
extern bool ChannelsLoaded;


/* Prototypes */
const char *svn_version(void);
void bot_start(void);
void *bot_shutdown(void *arg);

void db_setup(void);
void db_thread_init( void );
void db_load_servers(void);
void db_load_channels(void);
void db_add_logentry( IRCChannel_t *channel, char *nick, IRCMsgType_t msgType, 
                      char *text, bool extract );
void db_update_nick( IRCChannel_t *channel, char *nick, bool present, 
                     bool extract );
void db_flush_nicks( IRCChannel_t *channel );
void db_flush_nick( IRCServer_t *server, char *nick, IRCMsgType_t type, 
                    char *text, char *newNick );
bool db_check_nick_notify( IRCChannel_t *channel, char *nick, int hours );
void db_notify_nick( IRCChannel_t *channel, char *nick );
char *db_get_seen( IRCChannel_t *channel, char *nick, bool privmsg );
char *db_get_setting( char *name );
void db_set_setting( char *name, char *format, ... );
void db_check_schema( char *setting, char *desc, int codeSupports, 
                      QueryTable_t *defSchema, int defSchemaCount,
                      SchemaUpgrade_t *schemaUpgrade );
void db_check_schema_main(void);
void db_nick_history( IRCChannel_t *channel, char *nick, NickHistory_t type ); 
AuthData_t *db_get_auth( char *nick );
void db_set_auth( char *nick, AuthData_t *auth );
void db_free_auth( AuthData_t *auth );
void db_check_plugins( PluginDef_t *plugins, int count );

IRCChannel_t *FindChannelNum( IRCServer_t *server, int channum );
IRCChannel_t *FindChannel(IRCServer_t *server, const char *channame);
IRCServer_t *FindServerNum( int serverId );

void botCmd_initialize( void );
void botCmd_add( const char **command, BotCmdFunc_t func,
                 BotCmdHelpFunc_t helpFunc, void *tag );
void regexpBotCmdAdd( IRCServer_t *server, IRCChannel_t *channel );
int botCmd_parse( IRCServer_t *server, IRCChannel_t *channel, char *who, 
                  char *msg );
void botCmd_remove( char *command );
char *botCmdDepthFirst( BalancedBTreeItem_t *item, bool filterPlugins );
char *CommandLineParse( char *msg, char **pLine );

void send_notice( IRCChannel_t *channel, char *nick );
void notify_start(void);

BalancedBTree_t *db_get_plugins( void );
void plugins_initialize( void );
bool pluginLoad( char *name );
bool pluginUnload( char *name );
void pluginUnloadAll( void );

void regexp_initialize( void );
void regexp_add( const char *channelRegexp, const char *contentRegexp, 
                 RegexpFunc_t func, void *tag );
void regexp_remove( char *channelRegexp, char *contentRegexp );
void regexp_parse( IRCServer_t *server, IRCChannel_t *channel, char *who, 
                   char *msg, IRCMsgType_t type );
char *regexp_substring( char *msg, int *ovector, int stringcount,
                        int stringnum );

void LoggedChannelMessage( IRCServer_t *server, IRCChannel_t *channel,
                           char *message );
void LoggedActionMessage( IRCServer_t *server, IRCChannel_t *channel,
                          char *message );

void authenticate_start(void);
void authenticate_state_machine( IRCServer_t *server, IRCChannel_t *channel,
                                 char *nick, char *msg, void *tag );
bool authenticate_check( IRCServer_t *server, char *nick );

void logging_initialize( void );
void logging_toggle_debug( int signum, void *info, void *secret );
void LogOutputAdd( int fd, LogFileType_t type, void *identifier );
bool LogOutputRemove( LogFileChain_t *logfile );
bool LogSyslogAdd( int facility );
bool LogStdoutAdd( void );
bool LogFileAdd( char * filename );
bool LogFileRemove( char *filename );
void LogItemOutput( void *vitem );
void LogFlushOutput( void );

void thread_create( pthread_t *pthreadId, void * (*routine)(void *), 
                    void *arg, char *name, SigFunc_t sighupFunc, 
                    void *sighupArg );
void thread_register( pthread_t *pthreadId, char *name, SigFunc_t sighupFunc,
                      void *sighupArg );
char *thread_name( pthread_t pthreadId );
void thread_deregister( pthread_t pthreadId );
void ThreadAllKill( int signum );
SigFunc_t ThreadGetHandler( pthread_t threadId, int signum, void **parg );

void *transmit_thread(void *arg);
void transmitMsg( IRCServer_t *server, TxType_t type, char *channel, 
                  char *message );

void db_queue_query( int queryId, QueryTable_t *queryTable,
                     MYSQL_BIND *queryData, int queryDataCount,
                     QueryResFunc_t queryCallback, void *queryCallbackArg,
                     pthread_mutex_t *queryMutex );

void bind_null_blob( MYSQL_BIND *data, void *value );
void bind_string( MYSQL_BIND *data, char *value, enum enum_field_types type );
void bind_numeric( MYSQL_BIND *data, long long int value, 
                   enum enum_field_types type );

#if ( MYSQL_VERSION_ID < 40100 ) 
unsigned long mysql_get_server_version(MYSQL *mysql);
#endif

#if ( MYSQL_VERSION_ID < 40000 )
my_bool mysql_thread_init(void);
#endif

void do_backtrace( int signum, void *ip, void *arg );


#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
