<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<xsl:variable name="lang" select="'en'"/>

<xsl:variable name="thread" select="'Thread'"/>
<xsl:variable name="subject" select="'Subject'"/>
<xsl:variable name="author" select="'Author'"/>
<xsl:variable name="date" select="'Date'"/>
<xsl:variable name="to" select="'To'"/>
<xsl:variable name="cc" select="'CC'"/>
<xsl:variable name="attachments" select="'Attachments'"/>

<xsl:variable name="tree-context" select="'This message is part of the following thread:'"/>
<xsl:variable name="full-tree" select="'the complete thread tree sorted by date'"/>
<xsl:variable name="mail-appears-in" select="'This message was posted to the following mailing lists:'"/>
<xsl:variable name="thread-appears-in" select="'This thread has appeared on the following mailing lists:'"/>
<xsl:variable name="list-info" select="'Mailing List Info'"/>
<xsl:variable name="near-message" select="'Nearby Messages'"/>
<xsl:variable name="old-topics" select="'Old-Topics'"/>
<xsl:variable name="new-topics" select="'New-Topics'"/>
<xsl:variable name="raw-email" select="'Entire message'"/>

<xsl:variable name="front-page" select="'Starting Page'"/>
<xsl:variable name="search-menu" select="'Search the archive for matching messages'"/>
<xsl:variable name="list" select="'List'"/>
<xsl:variable name="group" select="'Group'"/>
<xsl:variable name="all-lists" select="'Any mailing list'"/>
<xsl:variable name="all-groups" select="'Any mailing list group'"/>
<xsl:variable name="search" select="'Search'"/>

<xsl:variable name="jump-to-date" select="'Jump to those messages within this mailing list which are nearest to the following date:'"/>
<xsl:variable name="search-list" select="'Search for messages within this mailing list which contain the following keywords:'"/>
<xsl:variable name="jump" select="'Jump'"/>
<xsl:variable name="use-special-word-list" select="'Use the keyword '"/>
<xsl:variable name="to-search-list" select="' to restrict your search to this mailing list.'"/>
<xsl:variable name="newest-messages" select="'View Newest Messages'"/>
<xsl:variable name="jump-group" select="'Jump to Group'"/>
<xsl:variable name="post-new" select="'Start a New Thread'"/>
<xsl:variable name="subscribe" select="'Visit Subscription Page'"/>
<xsl:variable name="recent-poster" select="'Latest Poster'"/>
<xsl:variable name="new-threads" select="'Active Threads'"/>
<xsl:variable name="post-count" select="'&#x2211;'"/>
<xsl:variable name="activity-chart" select="'Activity'"/>

<xsl:variable name="search-thread" select="'Search for messages within this thread which contain the following keywords:'"/>
<xsl:variable name="use-special-word-thread" select="'Use the keyword '"/>
<xsl:variable name="to-search-thread" select="' to restrict your search to this thread.'"/>

<xsl:variable name="refine-search" select="'Refine the search results to only those messages containing the following keywords:'"/>
<xsl:variable name="jump-search" select="'Jump to those matching messages which are nearest to the following date:'"/>
<xsl:variable name="no-refine" select="'The result set will remain unchanged. Only the displayed date-range of the messages is affected.'"/>
<xsl:variable name="useful-prefixs" select="'Search terms may include the prefixes au: sb: ml: to match author subject and mailing list respectively.'"/>

<xsl:variable name="jan">January</xsl:variable>
<xsl:variable name="feb">February</xsl:variable>
<xsl:variable name="mar">March</xsl:variable>
<xsl:variable name="apr">April</xsl:variable>
<xsl:variable name="may">May</xsl:variable>
<xsl:variable name="jun">June</xsl:variable>
<xsl:variable name="jul">July</xsl:variable>
<xsl:variable name="aug">August</xsl:variable>
<xsl:variable name="sep">September</xsl:variable>
<xsl:variable name="oct">October</xsl:variable>
<xsl:variable name="nov">November</xsl:variable>
<xsl:variable name="dec">December</xsl:variable>

<xsl:variable name="unknown-address" select="'Someone'"/>
<xsl:variable name="posted-at" select="' at '"/>
<xsl:variable name="admin-by" select="'Administrated by:'"/>
<xsl:variable name="version" select="'version'"/>

</xsl:stylesheet>
