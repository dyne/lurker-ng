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
  <td nowrap="NOWRAP" class="clipped">
   <div class="squash">
    <a href="../message/{summary/id}.{$ext}">
     <xsl:value-of select="summary/subject"/>
    </a>
    <!-- make this the same height as thread -->
    <img src="../imgs/a.png" width="1" height="24" alt=""/>
   </div>
  </td>
  <td nowrap="NOWRAP" class="clipped">
   <div class="squash">
    <xsl:apply-templates mode="email-name" select="summary/email"/>
   </div>
  </td>
  <td nowrap="NOWRAP">
   <xsl:apply-templates mode="date" select="summary"/>
  </td>
 </xsl:element>
 <xsl:text>&#10;</xsl:text>
</xsl:template>


<!-- Format a mindex request -->
<xsl:template match="mindex">
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
     <tr><th colspan="3"><xsl:value-of select="$jump-to-date"/></th></tr>
     <tr>
      <td>
       <xsl:choose>
        <xsl:when test="prev">
         <a href="{list/id}@{prev}.{$ext}"><img alt="&lt;-" src="../imgs/prev.png"/></a>
        </xsl:when>
        <xsl:otherwise>
         <img src="../imgs/a.png" alt=".."/>
        </xsl:otherwise>
       </xsl:choose>
      </td>
      <td width="100%" nowrap="NOWRAP" align="center">
       <form action="{server/cgi-url}/jump.cgi">
        <input type="hidden" name="doc-url" value="{server/doc-url}"/>
        <input type="hidden" name="format" value="{$ext}"/>
        <input type="hidden" name="list" value="{list/id}"/>
        
        <xsl:call-template name="date-fields">
         <xsl:with-param name="date" select="row[position()=11]/summary/id"/>
        </xsl:call-template>
        <input type="submit" value="{$jump}!"/>
       </form>
      </td>
      <td align="right">
       <xsl:choose>
        <xsl:when test="next">
         <a href="{list/id}@{next}.{$ext}"><img alt="-&gt;" src="../imgs/next.png"/></a>
        </xsl:when>
        <xsl:otherwise>
         <img src="../imgs/a.png" alt=".."/>
        </xsl:otherwise>
       </xsl:choose>
      </td>
     </tr>
     <tr>
      <td colspan="3" align="center">
       [ <a href="../list/{list/id}.{$ext}"><xsl:value-of select="$list-info"/></a> ]
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
     <col width="60%" align="left"/>
     <col width="30%" align="left"/>
     <col width="10%" align="right"/>
     <tr>
      <th><xsl:value-of select="$subject"/></th>
      <th><xsl:value-of select="$author"/></th>
      <th><xsl:value-of select="$date"/>&#160;(UTC)</th>
     </tr>
     <xsl:apply-templates mode="message" select="row"/>
    </table>
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
