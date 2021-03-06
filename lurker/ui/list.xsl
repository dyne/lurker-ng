<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<xsl:import href="common.xsl"/>

<!-- Format a new thread row -->
<xsl:template name="max">
 <xsl:param name="args"/>
 <xsl:choose>
  <xsl:when test="count($args) = 0">1</xsl:when>
  <xsl:otherwise>
   <xsl:variable name="first" select="$args[position()=1]"/>
   <xsl:variable name="best">
    <xsl:call-template name="max">
     <xsl:with-param name="args" select="$args[position() &gt; 1]"/>
    </xsl:call-template>
   </xsl:variable>
   <xsl:choose>
    <xsl:when test="$best &gt; $first"><xsl:value-of select="$best"/></xsl:when>
    <xsl:otherwise><xsl:value-of select="$first"/></xsl:otherwise>
   </xsl:choose>
  </xsl:otherwise>
 </xsl:choose>
</xsl:template>
<xsl:template match="row" mode="newthreads">
 <xsl:element name="tr">
  <xsl:attribute name="onClick">
   <xsl:text>self.location='</xsl:text>
   <xsl:value-of select="../server/doc-url"/>
   <xsl:choose>
    <xsl:when test="sum(day) &lt;= 1"><xsl:text>/message/</xsl:text></xsl:when>
    <xsl:otherwise><xsl:text>/thread/</xsl:text></xsl:otherwise>
   </xsl:choose>
   <xsl:value-of select="summary/id"/>
   <xsl:text>.</xsl:text>
   <xsl:value-of select="$ext"/>
   <xsl:text>';</xsl:text>
  </xsl:attribute>
  <xsl:attribute name="onDblClick">
   <xsl:text>self.location='</xsl:text>
   <xsl:value-of select="../server/doc-url"/>
   <xsl:text>/message/</xsl:text>
   <xsl:value-of select="summary/id"/>
   <xsl:text>.</xsl:text>
   <xsl:value-of select="$ext"/>
   <xsl:text>';</xsl:text>
  </xsl:attribute>
  <xsl:attribute name="onMouseOver">rollIn(this);</xsl:attribute>
  <xsl:attribute name="onMouseOut">rollOut(this);</xsl:attribute>
  <xsl:attribute name="class">
   <xsl:choose>
    <xsl:when test="@selected">lit</xsl:when>
    <xsl:when test="(position() mod 2) = 0">row1</xsl:when>
    <xsl:otherwise>row2</xsl:otherwise>
   </xsl:choose>
  </xsl:attribute>
  <td nowrap="NOWRAP">
   <div class="squash" onclick="this.firstChild.removeAttribute('href');">
   <xsl:choose>
    <xsl:when test="sum(day) &lt;= 1">
     <a href="../message/{summary/id}.{$ext}"><xsl:value-of select="title"/></a>
    </xsl:when>
    <xsl:otherwise>
     <a href="../thread/{summary/id}.{$ext}"><xsl:value-of select="title"/></a>
    </xsl:otherwise>
   </xsl:choose>
   </div>
  </td>
  <td nowrap="NOWRAP">
   <div class="squash">
    <xsl:apply-templates mode="email-name" select="summary/email"/>
   </div>
  </td>
  <td nowrap="NOWRAP" class="chart">
   <xsl:variable name="maxval">
    <xsl:call-template name="max">
     <xsl:with-param name="args" select="day"/>
    </xsl:call-template>
   </xsl:variable>
   <xsl:for-each select="day">
    <img src="../imgs/bar.png" height="{floor((number(.)*21 div $maxval)+1)}" width="5" alt=""/>
   </xsl:for-each>
  </td>
  <td align="right">
   <xsl:value-of select="sum(day)"/>
  </td>
 </xsl:element>
 <xsl:text>&#10;</xsl:text>
</xsl:template>


<!-- Format a list request -->
<xsl:template match="list">
 <html lang="{$lang}">
  <head>
   <link rel="stylesheet" href="../ui/default.css" type="text/css"/>
   <title><xsl:value-of select="list/email/@name"/></title>
   <script type="text/javascript" src="../ui/common.js"/>
  </head>
  <body>
   <div class="header">
    <table class="external">
     <tr>
      <td align="left"><h1><xsl:value-of select="list/email/@name"/></h1></td>
      <td align="right"><xsl:call-template name="navbar"/></td>
     </tr>
    </table>
    
    <table class="navigation">
     <tr><th align="left" colspan="2"><xsl:value-of select="$jump-to-date"/></th></tr>
     <tr>
      <td>
        <!-- make this the same height as mindex -->
        <img src="../imgs/a.png" width="1" height="24" alt=""/>
      </td>
      <td nowrap="NOWRAP" align="center" width="100%">
       <form action="{server/cgi-url}/jump.cgi" onsubmit="form_timezone(this)">
        <input type="hidden" name="doc-url" value="{server/doc-url}"/>
        <input type="hidden" name="format" value="{$ext}"/>
        <input type="hidden" name="list" value="{list/id}"/>
        
        <xsl:call-template name="date-fields">
         <xsl:with-param name="date" select="$jump-date"/>
        </xsl:call-template>
        <input type="submit" value="{$jump-button}"/>
       </form>
      </td>
     </tr>
     <tr>
      <td colspan="2" align="center">
       [ <a href="../mindex/{list/id}@{$last-date}.{$ext}"><xsl:value-of select="$newest-messages"/></a> ]
       [ <a href="../splash/index.{$ext}#{list/group}"><xsl:value-of select="$jump-group"/></a> ]
       <xsl:if test="list/email/@address">
        [ <a href="mailto:{list/email/@address}"><xsl:value-of select="$post-new"/></a> ]
       </xsl:if>
       <xsl:if test="list/link">
        [ <a href="{list/link}"><xsl:value-of select="$subscribe"/></a> ]
       </xsl:if>
      </td>
     </tr>
    </table>
   </div>
   
   
   <div class="body">
    <div class="mozbug">
     <table class="index squash">
      <col width="60%" align="left"/>
      <col width="30%" align="left"/>
      <col width="85"  align="left"/>
      <col width="40"  align="right"/>
      <tr>
       <th align="left"><xsl:value-of select="$new-threads"/></th>
       <th align="left"><xsl:value-of select="$recent-poster"/></th>
       <th align="left"><xsl:value-of select="$activity-chart"/></th>
       <th align="right"><xsl:value-of select="$post-count"/></th>
      </tr>
      <xsl:apply-templates mode="newthreads" select="row"/>
     </table>
    </div>
    
<!-- this annoys me
    <br/>
    
    <xsl:if test="list/description">
     <h2><xsl:value-of select="list/description"/></h2>
    </xsl:if>
-->
   </div>
   
   
   <div class="footer">
    <table class="navigation">
     <tr><th align="left" colspan="2"><xsl:value-of select="$search-list"/></th></tr>
     <tr>
      <td>
        <!-- make this the same height as mindex -->
        <img src="../imgs/a.png" width="1" height="24" alt=""/>
      </td>
      <td nowrap="NOWRAP" align="center">
       <form action="{server/cgi-url}/keyword.cgi" accept-charset="UTF-8">
        <input type="hidden" name="doc-url" value="{server/doc-url}"/>
        <input type="hidden" name="format"  value="{$ext}"/>
        <input type="text"   name="query"   value="ml:{list/id} " class="longtext"/>
        <input type="submit" name="submit"  value="{$search-button}"/>
       </form>
      </td>
     </tr>
     <tr>
      <td colspan="2" align="center">
       <xsl:value-of select="$use-special-word-list"/>
       <b>ml:<xsl:value-of select="list/id"/></b>
       <xsl:value-of select="$to-search-list"/>
      </td>
     </tr>
    </table>
    <xsl:call-template name="lurker-signature"/>
   </div>
  </body>
 </html>
</xsl:template>

</xsl:stylesheet>
