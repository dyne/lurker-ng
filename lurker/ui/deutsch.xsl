<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<xsl:variable name="lang" select="'de'"/>

<xsl:variable name="thread" select="'Thread'"/>
<xsl:variable name="subject" select="'Betreff'"/>
<xsl:variable name="author" select="'Autor'"/>
<xsl:variable name="date" select="'Datum'"/>
<xsl:variable name="to" select="'To'"/>
<xsl:variable name="cc" select="'CC'"/>
<xsl:variable name="attachments" select="'Anhänge'"/>

<xsl:variable name="tree-context" select="'Diese Nachricht ist Teil des folgenden Threads:'"/>
<xsl:variable name="full-tree" select="'Der komplette Thread sortiert nach Datum'"/>
<xsl:variable name="mail-appears-in" select="'Diese Nachricht wurde auf der folgenden MailingList gepostet:'"/>
<xsl:variable name="thread-appears-in" select="'Dieser Thread ist auf der folgenden MailingList aufgetaucht:'"/>
<xsl:variable name="list-info" select="'MailingList Infos'"/>
<xsl:variable name="near-message" select="'Nachrichten um die Zeit'"/>
<xsl:variable name="old-topics" select="'Alte Treads'"/>
<xsl:variable name="new-topics" select="'Neue Treads'"/>
<xsl:variable name="raw-email" select="'Komplette Nachricht'"/>

<xsl:variable name="front-page" select="'Start Seite'"/>
<xsl:variable name="search-menu" select="'Durchsuche das Archiv nach passenden Nachrichten'"/>
<xsl:variable name="list" select="'Liste'"/>
<xsl:variable name="group" select="'Gruppe'"/>
<xsl:variable name="all-lists" select="'Alle MailingLists'"/>
<xsl:variable name="all-groups" select="'Alle MailingList Gruppen'"/>
<xsl:variable name="search" select="'Suchen'"/>

<xsl:variable name="jump-to-date" select="'Springe zu Nachrichten nahe dem folgenden Datum innerhalb dieser MailingList:'"/>
<xsl:variable name="search-list" select="'Suche nach Nachrichten innerhalb dieser MailingList die folgende Stichwörter enthalten:'"/>
<xsl:variable name="jump" select="'Springe'"/>
<xsl:variable name="use-special-word-list" select="'Benutze das Stichwort '"/>
<xsl:variable name="to-search-list" select="' um die Suche auf diese MailingList zu beschränken.'"/>
<xsl:variable name="newest-messages" select="'Zeige die neuesten Nachrichten'"/>
<xsl:variable name="jump-group" select="'Springe zur Gruppe'"/>
<xsl:variable name="post-new" select="'Beginne einen neuen Thread'"/>
<xsl:variable name="subscribe" select="'Zur Anmeldungs-Seite'"/>
<xsl:variable name="recent-poster" select="'Letzter Poster'"/>
<xsl:variable name="new-threads" select="'Aktive Threads'"/>
<xsl:variable name="post-count" select="'&#x2211;'"/>
<xsl:variable name="activity-chart" select="'Aktivität'"/>

<xsl:variable name="search-thread" select="'Suche nach Nachrichten innerhalb dieses Threads die folgende Schlüsselwööer benthalten:'"/>
<xsl:variable name="use-special-word-thread" select="'Benutze das Stichwort '"/>
<xsl:variable name="to-search-thread" select="' um die Suche auf diesen Thread zu beschränken.'"/>

<xsl:variable name="refine-search" select="'In den gefundenen Nachrichten nach folgenden Stichwort suchen:'"/>
<xsl:variable name="jump-search" select="'Springe zu den Nachrichten um das folgenden Datum:'"/>
<xsl:variable name="no-refine" select="'Das Ergebnis bleibt unverändert. Nur das angezeigte Datum ist betroffen.'"/>
<xsl:variable name="useful-prefixs" select="'Such-Strings können die Zusätze au: sb: und ml: enthalten, um speziell nach Autor, Betreff und MailingList zu suchen.'"/>

<xsl:variable name="jan">Januar</xsl:variable>
<xsl:variable name="feb">Februar</xsl:variable>
<xsl:variable name="mar">März</xsl:variable>
<xsl:variable name="apr">April</xsl:variable>
<xsl:variable name="may">Mai</xsl:variable>
<xsl:variable name="jun">Juni</xsl:variable>
<xsl:variable name="jul">Juli</xsl:variable>
<xsl:variable name="aug">August</xsl:variable>
<xsl:variable name="sep">September</xsl:variable>
<xsl:variable name="oct">Oktober</xsl:variable>
<xsl:variable name="nov">November</xsl:variable>
<xsl:variable name="dec">Dezember</xsl:variable>

<xsl:variable name="unknown-address" select="'unbekannt'"/>
<xsl:variable name="posted-at" select="' am '"/>
<xsl:variable name="admin-by" select="'Administriert von:'"/>
<xsl:variable name="version" select="'Version'"/>
</xsl:stylesheet>
