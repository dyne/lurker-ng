<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<xsl:import href="common.xsl"/>

<!-- Format a new thread row -->
<xsl:template name="max">
 <xsl:param name="args"/>
 <xsl:variable name="rest"  select="$args[position() &gt; 1]"/>
 <xsl:variable name="first">
  <xsl:value-of select="$args[position()=1]"/>
 </xsl:variable>
 <xsl:variable name="best">
  <xsl:choose>
   <xsl:when test="count($args) &gt; 0">
    <xsl:call-template name="max">
     <xsl:with-param name="args" select="$rest"/>
    </xsl:call-template>
   </xsl:when>
   <xsl:otherwise>0</xsl:otherwise>
  </xsl:choose>
 </xsl:variable>
 <xsl:choose>
  <xsl:when test="$best &gt; $first"><xsl:value-of select="$best"/></xsl:when>
  <xsl:otherwise><xsl:value-of select="$first"/></xsl:otherwise>
 </xsl:choose>
</xsl:template>
<xsl:template match="row" mode="newthreads">
 <xsl:element name="tr">
  <xsl:attribute name="class">
   <xsl:choose>
    <xsl:when test="@selected">lit</xsl:when>
    <xsl:when test="(position() mod 2) = 0">row1</xsl:when>
    <xsl:otherwise>row2</xsl:otherwise>
   </xsl:choose>
  </xsl:attribute>
  <td nowrap="NOWRAP" class="clipped">
   <div class="squash">
    <a href="../thread/{summary/id}.{$ext}">
     <xsl:value-of select="summary/subject"/>
    </a>
   </div>
  </td>
  <td nowrap="NOWRAP" class="clipped">
   <div class="squash">
    <xsl:apply-templates mode="email-link" select="summary/email"/>
   </div>
  </td>
  <td nowrap="NOWRAP" class="chart">
   <xsl:variable name="maxval">
    <xsl:call-template name="max">
     <xsl:with-param name="args" select="day"/>
    </xsl:call-template>
   </xsl:variable>
   <xsl:for-each select="day">
    <img src="../imgs/bar.png" height="{(number(.)*21 div $maxval)+1}" width="5" alt=""/>
   </xsl:for-each>
  </td>
  <td align="right">
   <xsl:value-of select="sum(day)"/>
  </td>
 </xsl:element>
</xsl:template>


<!-- Format a list request -->
<xsl:template match="list">
 <html lang="{$lang}">
  <head>
   <link rel="stylesheet" href="../fmt/default.css" type="text/css"/>
   <title><xsl:value-of select="list/email/@name"/></title>
  </head>
  <body>
   <div class="header">
    <table class="external">
     <tr>
      <td align="left"><h1><xsl:value-of select="list/email/@name"/></h1></td>
      <td align="right">
       <h1><xsl:apply-templates mode="splash-link" select="server"/></h1>
      </td>
     </tr>
    </table>
    
    <table class="navigation">
     <tr><th colspan="2"><xsl:value-of select="$jump-to-date"/></th></tr>
     <tr>
      <td>
        <!-- make this the same height as mindex -->
        <img src="../imgs/a.png" width="1" height="24" alt=""/>
      </td>
      <td nowrap="NOWRAP" align="center" width="100%">
       <form action="{server/cgi-url}/jump.cgi">
        <input type="hidden" name="doc-url" value="{server/doc-url}"/>
        <input type="hidden" name="format" value="{$ext}"/>
        <input type="hidden" name="list" value="{list/id}"/>
        
        <xsl:call-template name="date-fields">
         <xsl:with-param name="date" select="$jump-date"/>
        </xsl:call-template>
        <input type="submit" value="{$jump}!"/>
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
    <table class="index">
     <col width="2*" align="left"/>
     <col width="1*" align="left"/>
     <col width="0*" align="right"/>
     <col width="0*" align="right"/>
     <tr>
      <th><xsl:value-of select="$new-threads"/></th>
      <th><xsl:value-of select="$recent-poster"/></th>
      <th><xsl:value-of select="$activity-chart"/></th>
      <th><xsl:value-of select="$post-count"/></th>
     </tr>
     <xsl:apply-templates mode="newthreads" select="row"/>
    </table>
    
<!-- this annoys me
    <br/>
    
    <xsl:if test="list/description">
     <h2><xsl:value-of select="list/description"/></h2>
    </xsl:if>
-->
   </div>
   
   
   <div class="footer">
    <table class="navigation">
     <tr><th colspan="2"><xsl:value-of select="$search-list"/></th></tr>
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
        <input type="submit" name="submit"  value="{$search}!"/>
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
