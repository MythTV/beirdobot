<?php
/**
 * IRC Channels
 *
 * @url         $URL$
 * @date        $Date$
 * @version     $Revision$
 * @author      $Author$
 *
 * @package     Beirdobot
 *
 * @uses        includes/irc_messages.php
 * @uses        includes/irc_servers.php
 * @uses        $Servers
 *
/**/

/**
 * @global  array   $GLOBALS['Channels']
 * @name    $Channels
/**/
    $Channels = array();

// Load all of the servers
    $sh = $db->query('SELECT channels.*,
                             MAX(irclog.timestamp) AS last_entry,
                             MIN(irclog.timestamp) AS first_entry
                        FROM channels
                             NATURAL LEFT JOIN irclog
                    GROUP BY chanid');
    while ($row = $sh->fetch_assoc()) {
        $Channels[$row['chanid']] = new irc_channel($row);
    }
    $sh->finish();


/**
 * Class to hold the IRC channels that are being logged.
/**/
class irc_channel {

    var $chanid;
    var $serverid;
    var $channel;
    var $url;
    var $notifywindow;
    var $cmdChar;
    var $last_entry;
    var $first_entry;

    var $server;
    var $messages = array();
    var $users    = array();

/**
 * Object constructor
 *
 * @param array $channel_vars   Hash of channel vars from the database.
/**/
    function __construct($channel_vars) {
        global $Servers;
    // Assign the various channel vars
        $this->chanid       = $channel_vars['chanid'];
        $this->serverid     = $channel_vars['serverid'];
        $this->channel      = $channel_vars['channel'];
        $this->url          = $channel_vars['url'];
        $this->notifywindow = $channel_vars['notifywindow'];
        $this->cmdChar      = $channel_vars['cmdChar'];
        $this->last_entry   = $channel_vars['last_entry'];
        $this->first_entry  = $channel_vars['first_entry'];
    // Keep a reference to this channel's server
        $this->server       =& $Servers[$this->serverid];
    // Add this channel to its parent server
        $Servers[$this->serverid]->add_channel($this);
    }

/**
 * Placeholder constructor for php4 compatibility
 *
 * @param array $channel_vars   Hash of channel vars from the database.
/**/
    function &irc_channel($channel_vars) {
        return $this->__construct($channel_vars);
    }

/**
 * Load all of the messages from the requested channel that were sent between
 * $from and $to.
 *
 * @param int $from timestamp of the first message to load
 * @param int $to   timestamp of the last message to load (default: now)
/**/
    function load_messages($from, $to = NULL) {
        global $db;
    // Default the end time to now
        if (is_null($to))
            $to = time();
    // Load the messages
        $sh = $db->query('SELECT *
                            FROM irclog
                           WHERE chanid=?
                                 AND timestamp >= ?
                                 AND timestamp <= ?',
                         $this->chanid,
                         $from,
                         $to
                        );
        while ($row = $sh->fetch_assoc()) {
            $this->messages[$row['msgid']] = new irc_message($row);
        }
        $sh->finish();

    }

/**
 * Load all currently in this channel.
/**/
    function load_users($time=NULL) {
        global $db;
    // Default the end time to now
        if (is_null($time) || $time == time()) {
        // Load the messages
            $sh = $db->query('SELECT *
                                FROM nicks
                               WHERE chanid=?
                                     AND present = 1
                            ORDER BY nick',
                             $this->chanid
                            );
            while ($row = $sh->fetch_assoc()) {
                $this->users[$row['nick']] = $row;
            }
            $sh->finish();
        }
    // If not now, we need to do some heavier math
        else {
            $sh = $db->query('SELECT nicks.*,
                                     IF(SUM(IF(irclog.msgtype=6, 1, -1)) > 0, 1, 0) AS present
                                FROM nicks, irclog
                               WHERE irclog.nick = nicks.nick
                                     AND irclog.timestamp <= ?
                                     AND irclog.chanid = ?
                                     AND irclog.msgtype IN (6, 7)
                            GROUP BY irclog.nick',
                             $time,
                             $this->chanid
                            );
            while ($row = $sh->fetch_assoc()) {
                $this->users[$row['nick']] = $row;
            }
            $sh->finish();
        }
    }

}

