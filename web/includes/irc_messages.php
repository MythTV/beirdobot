<?php
/**
 * IRC Messages
 *
 * @url         $URL$
 * @date        $Date$
 * @version     $Revision$
 * @author      $Author$
 *
 * @package     Beirdobot
 *
 * @uses        includes/irc_channels.php
 * @uses        $Channels
 *
 **/

// Define some message-related contstants
    define('MSG_NORMAL', 0);
    define('MSG_ACTION', 1);
    define('MSG_TOPIC',  2);
    define('MSG_KICK',   3);
    define('MSG_MODE',   4);
    define('MSG_NICK',   5);
    define('MSG_JOIN',   6);
    define('MSG_PART',   7);
    define('MSG_QUIT',   8);

/**
 * Class to hold an individual IRC message
 **/
class irc_message {

    var $msgid;
    var $chanid;
    var $timestamp;
    var $nick;
    var $msgtype;
    var $raw_message;
    var $message;

    var $class;
    var $channel;

/**
 * Object constructor
 *
 * @param array $msg_vars   Hash of msg vars from the database.
 **/
    function __construct($msg_vars) {
        global $Channels;
    // Assign the various message vars
        $this->msgid        = $msg_vars['msgid'];
        $this->chanid       = $msg_vars['chanid'];
        $this->timestamp    = $msg_vars['timestamp'];
        $this->nick         = $msg_vars['nick'];
        $this->msgtype      = $msg_vars['msgtype'];
        $this->raw_message  = trim($msg_vars['message']);
    // Keep a reference to this channel's server
        $this->channel      =& $Channels[$this->chanid];
    // Make a printable version of the message text
        $this->parse_message();
    // Build a css class
        switch ($this->msgtype) {
            case MSG_NORMAL:
                $this->class = 'msg_normal';    break;
            case MSG_ACTION:
                $this->class = 'msg_action';    break;
            case MSG_TOPIC:
                $this->class = 'msg_topic';     break;
            case MSG_KICK:
                $this->class = 'msg_kick';      break;
            case MSG_MODE:
                $this->class = 'msg_mode';      break;
            case MSG_NICK:
                $this->class = 'msg_nick';      break;
            case MSG_JOIN:
                $this->class = 'msg_join';      break;
            case MSG_PART:
                $this->class = 'msg_part';      break;
            case MSG_QUIT:
                $this->class = 'msg_quit';      break;
        }
    }

/**
 * Placeholder constructor for php4 compatibility
 *
 * @param array $msg_vars   Hash of msg vars from the database.
 **/
    function &irc_message($msg_vars) {
        return $this->__construct($msg_vars);
    }

/**
 * Return a color code for this nick.
 **/
    function nick_color() {
        static $cache = array();
        if (empty($cache[$this->nick])) {
            $color = 0;
        // PHP 5 or PHP_Compat?
            if (function_exists('str_split')) {
                foreach (str_split($this->nick) as $char) {
                    $color += ord($char);
                }
            }
        // Do it manually
            else {
                for ($i = 0; $i < strlen($this->nick); $i++) {
                    $color += ord($this->nick[$i]);
                }
            }
        // Cache
            $cache[$this->nick] = ($color % 8) + 1;
        }
        return $cache[$this->nick];
    }


/**
 * Sets $this->message from $this->raw_message, and then performs some regex
 * magic to add links, etc.
 **/
    function parse_message() {
        $this->message = html_entities($this->raw_message);
    // Only interact with certain kinds of messages
        if (!in_array($this->msgtype, array(MSG_NORMAL, MSG_ACTION)))
            return;
    // Perform some basic tag and text substitutions
		static $reg = array(
                        // french spaces, last one Guillemet-left only if there is something before the space
                            '/(.) (?=\\?|:|;|!|\\302\\273)/' => '\\1&nbsp;\\2',
                        // french spaces, Guillemet-right
                            '/(\\302\\253) /'        => '\\1&nbsp;',
                        // Unescape special characters  (yes, the \\\\ actually matches a single \ character)
                            '/^\\\\(?=[*#;])/m'      => '',
                        // Insert htmlized versions of various dashes
                            '/ - /'                    => '&nbsp;&ndash; ', // N dash
                            '/(?<=\d)-(?=\d)(?!\d+-)(?![^<]*?(?:>|<\\/a>))/' => '&ndash;',        // N dash between individual sequences of numbers
                            '/ -- /'                   => '&nbsp;&mdash; ', // M dash
                        // Break up really long strings (that aren't inside of links)
                            '/(\S{80})(?![^<]*?(?:>|<\\/a>))/' => '$1 ',
                           );
    // Perform the replacements
        // Convert URL's to clickable links
        $this->message = preg_replace_callback(
                '#(\w+://\S+)#',
                function ($matches)
                {
                        return irc_message::_parse_link(strip_quote_slashes($matches[0]));
                },
                $this->message);
        // Add links to email addresses (does only a basic scan for valid email address formats)
        $this->message = preg_replace_callback(
                '#((?:
                      (?:[^<>\(\)\[\]\\.,;:\s@\"]+(?:\.[^<>\(\)\[\]\\.,;:\s@\"]+)*)
                      |
                      (?:"[^"]+")
                    ) @ (?:
                      (?:\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])
                      |
                      (?:[a-z\-0-9]+\.)+[a-z]+
                  ))#x',
                function ($matches)
                {
                        return encoded_mailto($matches[0]);
                },
                $this->message);

        $this->message = preg_replace(array_keys($reg), array_values($reg),
                                      $this->message);
    }

/**
 * Send in a link, and get a formatted href out (called by parse_message)
 *
 * @param   string  $link   the link to turn into an href
 *
 * @return  formatted href link
 **/
    function _parse_link($link) {
    // parse_message() runs htmlentities first, so undo a bit of that
        $str = '<a href="'.str_replace('&amp;', '&', $link).'" rel="nofollow">';
    // Long links mess up the page wrapping, since browsers don't know to wrap
    // on characters like & or +
        if (strlen($link) > 64) {
            $link = substr($link, 0, 45) . ' . . . ' . substr($link, - 12);
        }
    // Return
        return $str.$link.'</a>';
    }
}

