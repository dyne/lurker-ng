<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<xsl:import href="common.xsl"/>

<!-- Format the mailing lists -->
<xsl:template mode="appear-in" match="list">
 <tr>
  <td nowrap="NOWRAP" class="padded">
   <b><xsl:value-of select="email/@name"/></b><br/>
   <a href="../list/{id}.{$ext}"><xsl:value-of select="$list-info"/></a> |
   <a href="../mindex/{id}@{../row[position()=1]/summary/id}.{$ext}"><xsl:value-of select="$near-message"/></a>
  </td>
  <td><xsl:value-of select="description"/></td>
 </tr>
</xsl:template>

<!-- Format a message row -->
<xsl:template match="row" mode="message">
 <xsl:element name="tr">
  <xsl:attribute name="onClick">
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
   <a name="{summary/id}"/>
   <xsl:apply-templates mode="tree" select="tree"/>
  </td>
  <td nowrap="NOWRAP"><xsl:apply-templates mode="email-name" select="summary/email"/></td>
  <td nowrap="NOWRAP"><xsl:apply-templates mode="text-date" select="summary"/></td>
 </xsl:element>
 <xsl:text>&#10;</xsl:text>
</xsl:template>


<!-- Format a thread request -->
<xsl:template match="thread">
 <html lang="{$lang}">
  <head>
   <link rel="stylesheet" href="../fmt/default.css" type="text/css"/>
   <title><xsl:value-of select="row[position()=1]/summary/subject"/></title>
   <script type="text/javascript" src="../fmt/common.js"/>
  </head>
  <body>
   <div class="header">
    <table class="external">
     <tr>
      <td align="left"><h1><xsl:value-of select="row[position()=1]/summary/subject"/></h1></td>
      <td align="right">
       <h1><xsl:apply-templates mode="splash-link" select="server"/></h1>
      </td>
     </tr>
    </table>
    
    <table class="navigation">
     <tr>
      <th colspan="2" align="left">
       <xsl:value-of select="$thread-appears-in"/>
      </th>
     </tr>
     <xsl:apply-templates mode="appear-in" select="list"/>
    </table>
   </div>
   
   
   <div class="body">
    <div class="mozbug">
     <table class="index">
      <col width="60%" align="left"/>
      <col width="30%" align="left"/>
      <col width="10%" align="right"/>
      <tr>
       <th align="left"><xsl:value-of select="$thread"/></th>
       <th align="left"><xsl:value-of select="$author"/></th>
       <th align="left"><xsl:value-of select="$date"/>
           <xsl:text>&#160;(</xsl:text>
           <xsl:apply-templates select="row[position()=1]/summary" mode="timezone"/>
           <xsl:text>)</xsl:text>
       </th>
      </tr>
      <xsl:apply-templates mode="message" select="row"/>
     </table>
    </div>
   </div>
   
   
   <div class="footer">
    <table class="navigation">
     <tr><th align="left" colspan="2"><xsl:value-of select="$search-thread"/></th></tr>
     <tr>
      <td>
        <!-- make this the same height as mindex -->
        <img src="../imgs/a.png" width="1" height="24" alt=""/>
      </td>
      <td nowrap="NOWRAP" align="center">
       <form action="{server/cgi-url}/keyword.cgi" accept-charset="UTF-8">
        <input type="hidden" name="doc-url" value="{server/doc-url}"/>
        <input type="hidden" name="format"  value="{$ext}"/>
        <input type="text"   name="query"   value="th:{hash} " class="longtext"/>
        <input type="submit" name="submit"  value="{$search}!"/>
       </form>
      </td>
     </tr>
     <tr>
      <td colspan="2" align="center">
       <xsl:value-of select="$use-special-word-thread"/>
       <b>th:<xsl:value-of select="hash"/></b>
       <xsl:value-of select="$to-search-thread"/>
      </td>
     </tr>
    </table>
    <xsl:call-template name="lurker-signature"/>
   </div>
  </body>
 </html>
</xsl:template>

</xsl:stylesheet>
