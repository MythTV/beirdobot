<?php
/**
 * The Database class and associated statement libraries.
 *
 * @url       $URL: svn+ssh://xris@svn.siliconmechanics.com/var/svn/web/trunk/shared_code/includes/db.php $
 * @date      $Date: 2006-01-28 19:19:02 -0800 (Sat, 28 Jan 2006) $
 * @version   $Revision: 8758 $
 * @author    $Author: xris $
 * @license   GPL
 *
 * @package   MythWeb
 *
 * @uses      includes/errors.php
 * @uses      smart_args() from utils.php
 *
 **/


/**
 *  This should already be loaded by the time db.php is, but we should at least
 *  let other people know that this library is required.
 **/
    require_once 'includes/errors.php';

/**
 *  This should already be loaded by the time db.php is, but we should at least
 *  let other people know that this libary is required for the function
 *  smart_args
 **/
    require_once 'includes/utils.php';

/**
 *  This Database class is designed to be a database abstraction later, similar
 *  to perl's DBI library.  It currently only supports mysql, but future
 *  versions may be made to support mysqli, pgsql, etc.
 **/
class Database {

/** @var resource   Resource handle for this database connection */
    var $dbh;

/** @var string     A full error message generated by the coder */
    var $error;

/** @var string     The database-generated error string */
    var $err;

/** @var int        The database-generated error number */
    var $errno;

/** @var resource   The last statement handle created by this object */
    var $last_sh;

/** @var bool       This controls if the mysql query errors are fatal or just stored in the mysql error string */
    var $fatal_errors = true;

/**
 *  The regular expression used to see if a LIMIT statement exists within
 *  the current query.
 **/
    var $limit_regex = '/((.*)\sLIMIT\s+\d+(?:\s*(?:,|OFFSET)\s*\d+)?)?
                          ((?:\s+PROCEDURE\s+\w+\(.+?\))?
                           (?:\s+FOR\s+UPDATE)?
                           (?:\s+LOCK\s+IN\s+SHARE\s+MODE)?
                          )
                          \s*$/x';

/**
 *  The regular expression used to see if a LIMIT statement exists within
 *  the current query.
 **/
    var $limit_regex_replace = '"$2"." LIMIT 1 "."$3"';

/**
 * Database Constructor
 *
 *  @param string $db_name      Name of the database we're connecting to
 *  @param string $login        Login name to use when connecting
 *  @param string $password     Password to use when connecting
 *  @param string $server       Database server to connect to (Default: localhost)
 **/
    function __construct($db_name, $login, $password, $server='localhost') {
    // Connect to the database
    // For now, all we have is mysql -- maybe someday we get other stuff.
        $this->dbh = @mysqli_connect($server, $login, $password, $db_name)
            or $this->error("Can't connect to the database server.");
    }

/**
 *  Fill the error variables
 *
 *  @param string $error    The string to set the error message to.  Set to
 *                          false if you want to wipe out the existing errors.
 **/
    function error($error='') {
        if ($error === false) {
            $this->err   = null;
            $this->errno = null;
            $this->error = null;
        }
        else {
            $this->err   = $this->dbh ? mysqli_error($this->dbh) : mysqli_error();
            $this->errno = $this->dbh ? mysqli_errno($this->dbh) : mysqli_errno();
            $this->error = ($error ? "$error\n\n" : '')."$this->err [#$this->errno]";
        }
    }

/**
 *  Perform a database query and return a handle.  Usage:
 *
 *  <pre>
 *      $sh =& $db->query('SELECT * FROM foo WHERE x=? AND y=? AND z="bar\\?"',
 *                        $x_value, $y_value);
 *  </pre>
 *
 *  @param string $query    The query string
 *  @param mixed  $arg      Query arguments to escape and insert at ? placeholders in $query
 *  @param mixed  ...       Additional arguments
 *
 *  @return mixed           Statement handle for the current type of database connection
 **/
    function &query($query) {
    // Hack to get query_row and query_assoc working correctly
        $args = array_slice(func_get_args(), 1);
    // Split out sub-arrays, etc..
        $args = smart_args($args);
    // Create and return a database query
        $this->last_sh =& $this->prepare($query);
        $this->last_sh->execute($args);
    // PHP 5 doesn't like us returning NULL by reference
        if (!$this->last_sh->sh)
            $this->last_sh = NULL;
        return $this->last_sh;
    }

/**
 *  Returns a single row from the database and frees the result.
 *
 *  @param string $query    The query string
 *  @param mixed  $arg      Query arguments to escape and insert at ? placeholders in $query
 *  @param mixed  ...       Additional arguments
 *
 *  @return array
 **/
    function query_row($query) {
    // Add a "LIMIT 1" if no limit was specified -- this will speed up queries at least slightly
        $query = preg_replace($this->limit_regex, $this->limit_regex_replace, $query, 1);
    // Query and return
        $args   = array_slice(func_get_args(), 1);
        $sh     = $this->query($query, $args);
        if ($sh) {
            $return = $sh->fetch_row();
            $sh->finish();
            return $return;
        }
        return null;
    }

/**
 *  Returns a single assoc row from the database and frees the result.
 *
 *  @param string $query    The query string
 *  @param mixed  $arg      Query arguments to escape and insert at ? placeholders in $query
 *  @param mixed  ...       Additional arguments
 *
 *  @return assoc
 **/
    function query_assoc($query) {
    // Add a "LIMIT 1" if no limit was specified -- this will speed up queries at least slightly
        $query = preg_replace($this->limit_regex, $this->limit_regex_replace, $query, 1);
    // Query and return
        $args   = array_slice(func_get_args(), 1);
        $sh     = $this->query($query, $args);
        if ($sh) {
            $return = $sh->fetch_assoc();
            $sh->finish();
            return $return;
        }
        return null;
    }

/**
 *  Returns a single column from the database and frees the result.
 *
 *  @param string $query    The query string
 *  @param mixed  $arg      Query arguments to escape and insert at ? placeholders in $query
 *  @param mixed  ...       Additional arguments
 *
 *  @return mixed
 **/
    function query_col($query) {
    // Add a "LIMIT 1" if no limit was specified -- this will speed up queries at least slightly
        //$query        = preg_replace($this->limit_regex, $this->limit_regex_replace, $query, 1);
    // Query and return
        $args         = array_slice(func_get_args(), 1);
        $sh           = $this->query($query, $args);
        if ($sh) {
            list($return) = $sh->fetch_row();
            $sh->finish();
            return $return;
        }
        return null;
    }

/**
 *  Returns an un-executed Database_Query_mysql object
 *
 *  @param string $query    The query string
 *
 *  @return Database_Query_mysql
 **/
    function &prepare($query) {
        $new_query = new Database_Query_mysql($this, $query);
        return $new_query;
    }

/**
 *  Wrapper for the last query statement's insert_id method.
 *  @return int
 **/
    function insert_id() {
        return $this->last_sh->insert_id();
    }

/**
 *  Wrapper for the last query statement's affected_rows method.
 *  @return int
 **/
    function affected_rows() {
        return $this->last_sh->affected_rows();
    }


/**
 *  Escapes a string and returns it with added quotes.
 *  On top of normal escaping, this also escapes ? characters so it's safe to
 *  use in other db queries.
 *  @return string
 **/
    function escape($string) {
    // Null?
        if (is_null($string))
            return 'NULL';
    // Just a string
        return str_replace('?', '\\?', "'".mysqli_real_escape_string($string)."'");
    }

/**
 * This function and the next one control if the mysqli_query throws a fatal error or not
 **/
    function enable_fatal_errors() {
        $this->fatal_errors = true;
    }

/**
 * This function disables the fatal error trigger code
 **/
    function disable_fatal_errors() {
        $this->fatal_errors = false;
    }

}

/**
 * Parent class for all database query types.
 **/
class Database_Query {

/** @var resource   The related database connection handle */
    var $dbh = NULL;

/** @var class      The parent db class */

/** @var resource   The current active statement handle */
    var $sh = NULL;

/** @var array      The query string, broken apart where arguments should be inserted */
    var $query = array();

/** @var string     The most recent query sent to the server */
    var $last_query = '';

/** @var int        Number of arguments required by $query */
    var $num_args_needed = 0;

/**
 * Constructor.  Parses $query and splits it at ? characters for later
 *  substitution in execute().
 *
 *  @param Database $dbh    The parent Database object
 *  @param string   $query  The query string
 **/
    function __construct(&$db, $query) {
        $this->dbh             = $db->dbh;
        $this->db              =& $db;
        $this->num_args_needed = max(0, substr_count($query, '?') - substr_count($query, '\\?'));
    // Build an optimized version of the query
        if ($this->num_args_needed > 0) {
            $this->query = array();
            foreach (preg_split('/(\\\\?\\?)/', $query, -1, PREG_SPLIT_DELIM_CAPTURE) as $part) {
                switch ($part) {
                    case '?':
                        break;
                    case '\\?':
                        $this->query[min(0, count($this->query) - 1)] .= '?';
                        break;
                    default:
                        $this->query[] = $part;
                }
            }
        }
        else
            $this->query = array($query);
    }

}

/**
 *  The basic MySQL database query type.
 **/
class Database_Query_mysql extends Database_Query {

/**
 * Executes the query that was previously passed to the constructor.
 *
 *  @param mixed  $arg      Query arguments to escape and insert at ? placeholders in $query
 *  @param mixed  ...       Additional arguments
 *
 *  @return result
 **/
    function execute() {
    // Load the function arguments, minus the query itself, which we already extracted
        $args = func_get_args();
    // Split out sub-arrays, etc..
        $args = smart_args($args);
    // Were enough arguments passed in?
        if (count($args) != $this->num_args_needed)
            trigger_error('Database_Query called with '.count($args)." arguments, but requires $this->num_args_needed.", FATAL);
    // Finish any previous statements
        $this->finish();
    // Replace in the arguments
        $this->last_query = '';
        foreach ($this->query as $part) {
            $this->last_query .= $part;
            if (count($args)) {
                $arg = array_shift($args);
                $this->last_query .= is_null($arg)
                                        ? 'NULL'
                                        : "'".mysqli_real_escape_string($this->dbh, $arg)."'";
            }
        }
    // Perform the query
        $this->sh = mysqli_query($this->dbh, $this->last_query);
        if ($this->sh === false) {
            if ($this->db->fatal_errors)
                trigger_error('SQL Error: '.mysqli_error($this->dbh).' [#'.mysqli_errno($this->dbh).']', FATAL);
            else
                $this->db->error();
        }
    }

/**
 *  The following routines basically replicate the mysql functions built into
 *  php.  The only difference is that the resource handle gets passed-in
 *  automatically.  eg.
 *
 *      mysqli_fetch_row($result);   ->  $sh->fetch_row();
 *      mysqli_affected_rows($dbh);  ->  $sh->affected_rows();
 **/

/**
 *  Fetch a single column
 *  @return mixed
 **/
    function fetch_col() {
        list($return) = mysqli_fetch_row($this->sh);
        return $return;
    }

/**
 * Fetch a single row
 *  @link http://www.php.net/manual/en/function.mysql-fetch-row.php
 *  @return array
 **/
    function fetch_row() {
        return mysqli_fetch_row($this->sh);
    }

/**
 * Fetch a single assoc row
 *  @link http://www.php.net/manual/en/function.mysql-fetch-assoc.php
 *  @return assoc
 **/
    function fetch_assoc() {
        return mysqli_fetch_assoc($this->sh);
    }

/**
 * Fetch a single row as an array containing both numeric and assoc fields
 *  @link http://www.php.net/manual/en/function.mysql-fetch-array.php
 *  @return assoc
 **/
    function fetch_array($result_type=MYSQL_BOTH) {
        return mysqli_fetch_array($this->sh, $result_type);
    }

/**
 * Fetch a single row as an object
 *  @link http://www.php.net/manual/en/function.mysql-fetch-object.php
 *  @return object
 **/
    function fetch_object() {
        return mysqli_fetch_object($this->sh);
    }

/**
 *  @link http://www.php.net/manual/en/function.mysql-data-seek.php
 *  @return bool
 **/
    function data_seek($row_number) {
        return mysqli_data_seek($this->sh, $row_number);
    }

/**
 *  @link http://www.php.net/manual/en/function.mysql-num-rows.php
 *  @return int
 **/
    function num_rows() {
        return mysqli_num_rows($this->sh);
    }

/**
 *  @link http://www.php.net/manual/en/function.mysql-data-seek.php
 *  @return int
 **/
    function affected_rows() {
        return mysqli_affected_rows($this->dbh);
    }

/**
 *  @link http://www.php.net/manual/en/function.mysql-insert-id.php
 *  @return int
 **/
    function insert_id() {
        return mysqli_insert_id($this->dbh);
    }

/**
 * For anal people like me who like to free up memory manually
 **/
    function finish() {
        if ($this->sh && is_resource($this->sh))
            mysqli_free_result($this->sh);
        unset($this->sh);
    }

}

