<?php
/**
 * HTML Header
 *
 * @url         $URL$
 * @date        $Date$
 * @version     $Revision$
 * @author      $Author$
 *
 * @package     Beirdobot
 *
/**/
?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
<html>
<head>
    <title><?php echo $page_title ?></title>

    <script type="text/javascript" src="<?php echo root ?>js/utils.js"></script>
    <script type="text/javascript" src="<?php echo root ?>js/browser.js"></script>
    <script type="text/javascript" src="<?php echo root ?>js/visibility.js"></script>
    <script type="text/javascript" src="<?php echo root ?>js/ajax.js"></script>

    <link rel="stylesheet" type="text/css" href="<?php echo skin_url  ?>/global.css" >
    <link rel="stylesheet" type="text/css" href="//www.mythtv.org/css/site.css">
<?php
    if (!empty($headers))
        echo "\n    ", implode("\n    ", $headers), "\n";
?>
</head>

<body>

<!-- Header -->
<div id="header">
    <div id="header_logo">
        <a href="/"><img src="/img/mythtv.png" class="png" width="180" height="64" border="0" alt="MythTV"></a>
    </div>
    <div id="header_text">
        <ul>
            <li class="cur"><a href="/detail/mythtv">About MythTV</a>
                <div>
                <ul>
                    <li class="first"><a href="/detail/mythtv">MythTV In Detail</a></li>
                    <li><a href="/license">Licensing</a></li>
                    <li><a href="http://www.mythtv.org/wiki/Frequently_Asked_Questions">FAQ</a></li>
                    <li class="last"><a href="/contact">Contact Us</a></li>
                </ul>
                </div>
                </li>
            <li><a href="/support">Support</a>
                <div>
                <ul>
                    <li class="first"><a href="/support">Overview</a></li>
                    <li><a href="/docs/">Documentation</a></li>
                    <li><a href="https://forum.mythtv.org/">Forums</a></li>
                    <li><a href="http://wiki.mythtv.org/">Wiki</a></li>
                    <li><a href="https://code.mythtv.org/trac/wiki/TicketHowTo">Bugs</a></li>
                    <li class="last"><a href="https://code.mythtv.org/trac/">Development</a></li>
                </ul>
                </div>
                </li>
            <li><a href="/download">Download</a>
                </li>
            <li><a href="/news">News Archive</a>
                </li>
        </ul>
    </div>
    <div id="header-end"></div>
</div><!-- header -->

<!-- Page Content -->
<div id="wrap">
