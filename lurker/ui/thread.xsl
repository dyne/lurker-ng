<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<xsl:import href="common.xsl"/>


<!-- Format a message row -->
<xsl:template match="row" mode="message">
 <xsl:element name="tr">
  <xsl:attribute name="class">
   <xsl:choose>
    <xsl:when test="@selected">lit</xsl:when>
    <xsl:when test="(position() mod 2) = 0">row1</xsl:when>
    <xsl:otherwise>row2</xsl:otherwise>
   </xsl:choose>
  </xsl:attribute>
  <td nowrap="NOWRAP">
   <xsl:apply-templates mode="tree" select="tree"/>
  </td>
  <td nowrap="NOWRAP"><xsl:apply-templates mode="email-name" select="summary/email"/></td>
  <td nowrap="NOWRAP"><xsl:apply-templates mode="date" select="summary"/></td>
 </xsl:element>
 <xsl:text>&#10;</xsl:text>
</xsl:template>


<!-- Format a thread request -->
<xsl:template match="thread">
 <html lang="{$lang}">
  <head>
   <link rel="stylesheet" href="../fmt/default.css" type="text/css"/>
   <title><xsl:value-of select="row[position()=1]/summary/subject"/></title>
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
     <tr><th colspan="2"><xsl:value-of select="$search-thread"/></th></tr>
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
       <b>ml:<xsl:value-of select="hash"/></b>
       <xsl:value-of select="$to-search-thread"/>
      </td>
     </tr>
    </table>
   </div>
   
   
   <div class="body">
    <table class="index">
     <col width="60%" align="left"/>
     <col width="30%" align="left"/>
     <col width="10%" align="right"/>
     <tr>
      <th><xsl:value-of select="$thread"/></th>
      <th><xsl:value-of select="$author"/></th>
      <th><xsl:value-of select="$date"/>&#160;(UTC)</th>
     </tr>
     <xsl:apply-templates mode="message" select="row"/>
    </table>
   </div>
   
   
   <div class="footer">
<!-- fixme (have lists)
    <table class="navigation">
    </table>
-->
    <xsl:call-template name="lurker-signature"/>
   </div>
  </body>
 </html>
</xsl:template>

</xsl:stylesheet>
