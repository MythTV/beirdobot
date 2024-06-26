<?php
/**
 * Print information about a specific irc channel
 *
 * @url         $URL$
 * @date        $Date$
 * @version     $Revision$
 * @author      $Author$
 *
 * @package     Beirdobot
 *
 **/

/**
 * @global  mixed   $GLOBALS['Channel']
 * @name    $Channel
 **/
    $Channel = $Channels[$Path[1]];

// Unknown or empty channel
    if (empty($Path[1]) || empty($Channel)) {
        redirect_browser(root);
    }

// Channel history page?
    if (array_key_exists(2, $Path) && $Path[2] == 'history') {
        require_once 'modules/channel/history.php';
        exit;
    }

/**
 * @global  int     $GLOBALS['start']
 * @name    $start
 **/
    $start = null;
/**
 * @global  int     $GLOBALS['end']
 * @name    $end
 **/
    $end = null;

// Date?
    if (array_key_exists(2, $Path) &&
        preg_match('/^(\d+)-(\d+)-(\d+)(?::(\d+):(\d+)(?::(\d+))?)?$/', $Path[2], $match)) {
        if (empty($match[4])) {
            $match[4] = 0;
            $match[5] = 0;
        }
        if (empty($match[6]))
            $match[6] = 0;
        $start = mktime($match[4], $match[5], $match[6], $match[2], $match[3], $match[1]);
        if (array_key_exists(3, $Path) &&
            preg_match('/^(\d+)-(\d+)-(\d+)(?::(\d+):(\d+)(?::(\d+))?)?$/', $Path[3], $match)) {
            if (empty($match[4])) {
                $match[4] = 23;
                $match[5] = 59;
            }
            if (empty($match[6]))
                $match[6] = 59;
            $end = mktime($match[4], $match[5], $match[6], $match[2], $match[3], $match[1]);
        }
        else {
            $end = $start + day_in_seconds - 1;
        }
    }

// No start time -- show the most recent 15 minutes (or so)
    if (!$start) {
        $start = 60 * intVal((time() - (15 * 60)) / 60);
    }
// No end date
    elseif ($start && (!$end || $start == $end)) {
        $end = $start + day_in_seconds - 1;
    }
// Out of order?
    elseif ($start > $end) {
        $tmp   = $start;
        $start = $end;
        $end   = $tmp;
    }

// Load the names of the users currently logged into the channel
    $Channel->load_users($end);

// Print the page
    require 'templates/channel.php';
