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
    <xsl:apply-templates mode="email-link" select="summary/email"/>
   </div>
  </td>
  <td>
   <xsl:apply-templates mode="date" select="summary"/>
  </td>
 </xsl:element>
</xsl:template>


<!-- Format a search request -->
<xsl:template match="search">
 <xsl:variable name="sdate">
  <xsl:choose>
   <xsl:when test="row/summary/id"><xsl:value-of select="row/summary/id"/></xsl:when>
   <xsl:otherwise><xsl:value-of select="$jump-date"/></xsl:otherwise>
  </xsl:choose>
 </xsl:variable>
 
 <html lang="{$lang}">
  <head>
   <link rel="stylesheet" href="../fmt/default.css" type="text/css"/>
   <title><xsl:value-of select="$search"/>: <xsl:value-of select="translate(query,',',' ')"/></title>
  </head>
  <body>
   <div class="header">
    <table class="external">
     <tr>
      <td align="left">
       <h1><xsl:value-of select="$search"/>: <xsl:value-of select="translate(query,',',' ')"/></h1>
      </td>
      <td align="right">
       <h1><xsl:apply-templates mode="splash-link" select="server"/></h1>
      </td>
     </tr>
    </table>
    
    <table class="navigation">
     <tr><th colspan="3"><xsl:value-of select="$refine-search"/></th></tr>
     <tr>
      <td>
       <xsl:choose>
        <xsl:when test="prev">
         <a href="{prev}@{translate(query,'/','!')}.{$ext}"><img alt="&lt;-" src="../imgs/prev.png"/></a>
        </xsl:when>
        <xsl:otherwise>
         <img src="../imgs/a.png" alt=".."/>
        </xsl:otherwise>
       </xsl:choose>
      </td>
      <td width="100%" nowrap="NOWRAP" align="center">
       <form action="{server/cgi-url}/keyword.cgi" accept-charset="UTF-8">
        <input type="hidden" name="doc-url" value="{server/doc-url}"/>
        <input type="hidden" name="format"  value="{$ext}"/>
        <input type="hidden" name="sec"  value="{substring($sdate,14,2)}"/>
        <input type="hidden" name="min"  value="{substring($sdate,12,2)}"/>
        <input type="hidden" name="hour" value="{substring($sdate,10,2)}"/>
        <input type="hidden" name="mday" value="{substring($sdate,7,2)}"/>
        <input type="hidden" name="mon"  value="{substring($sdate,5,2)}"/>
        <input type="hidden" name="year" value="{substring($sdate,1,4)}"/>
        <input type="text"   name="query"   value="{translate(query,',',' ')}" class="longtext"/>
        <input type="submit" name="submit" value="{$search}"/>
       </form>
      </td>
      <td align="right">
       <xsl:choose>
        <xsl:when test="next">
         <a href="{next}@{translate(query,'/','!')}.{$ext}"><img alt="-&gt;" src="../imgs/next.png"/></a>
        </xsl:when>
        <xsl:otherwise>
         <img src="../imgs/a.png" alt=".."/>
        </xsl:otherwise>
       </xsl:choose>
      </td>
     </tr>
     <tr>
      <td colspan="3" align="center"><xsl:value-of select="$useful-prefixs"/></td>
     </tr>
    </table>
   </div>
   
   
   <div class="body">
    <table class="index">
     <col width="2*" align="left"/>
     <col width="1*" align="left"/>
     <col width="0*" align="right"/>
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
     <tr><th colspan="2"><xsl:value-of select="$jump-search"/></th></tr>
     <tr>
      <td>
        <!-- make this the same height as mindex -->
        <img src="../imgs/a.png" width="1" height="24"/>
      </td>
      <td nowrap="NOWRAP" align="center">
       <form action="{server/cgi-url}/keyword.cgi" accept-charset="UTF-8">
        <input type="hidden" name="doc-url" value="{server/doc-url}"/>
        <input type="hidden" name="format"  value="{$ext}"/>
        <input type="hidden" name="query"   value="{translate(query,',',' ')}"/>
        <xsl:call-template name="date-fields">
         <xsl:with-param name="date" select="$sdate"/>
        </xsl:call-template>
        <input type="submit" name="submit" value="{$jump}!"/>
       </form>
      </td>
     </tr>
     <tr>
      <td colspan="2" align="center"><xsl:value-of select="$no-refine"/></td>
     </tr>
    </table>
    <xsl:call-template name="lurker-signature"/>
   </div>
  </body>
 </html>
</xsl:template>

</xsl:stylesheet>
