<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    version="1.0">

<xsl:template match="search" mode="title">
 <xsl:value-of select="$titlesearch"/>: <xsl:value-of select="query"/>
</xsl:template>

<xsl:template match="search" mode="body">
 <h2>
  <xsl:choose>
   <xsl:when test="prev">
    <a href="{$first-date}@{query}.{$ext}"><img alt="&lt;&lt;=" src="../imgs/first.png"/></a>
    <a href="{prev}@{query}.{$ext}"><img alt="&lt;-" src="../imgs/prev.png"/></a>
   </xsl:when>
   <xsl:otherwise>
    <img src="../imgs/a.png" alt="..."/>
    <img src="../imgs/a.png" alt=".."/>
   </xsl:otherwise>
  </xsl:choose>
  <xsl:choose>
   <xsl:when test="next">
    <a href="{next}@{query}.{$ext}"><img alt="-&gt;" src="../imgs/next.png"/></a>
    <a href="{$last-date}@{query}.{$ext}"><img alt="=&gt;&gt;" src="../imgs/last.png"/></a>
   </xsl:when>
   <xsl:otherwise>
    <img src="../imgs/a.png" alt=".."/>
    <img src="../imgs/a.png" alt="..."/>
   </xsl:otherwise>
  </xsl:choose>
 
  <xsl:text>&#160;&#160;&#160;</xsl:text>
  
  <xsl:value-of select="$titlesearch"/>
 </h2>
 
 <xsl:variable name="sdate" select="row/summary/id"/>
 <div id="smSearch">
  <form action="{server/cgi-url}/keyword.cgi">
   <input type="hidden" name="doc-url" value="{server/doc-url}"/>
   <input type="hidden" name="format"  value="{$ext}"/>
   <input type="text"   name="query"   value="{translate(query,',',' ')}" class="longtext"/>
   <input type="hidden" name="sec"  value="substring($sdate,14,2)"/>
   <input type="hidden" name="min"  value="substring($sdate,12,2)"/>
   <input type="hidden" name="hour" value="substring($sdate,10,2)"/>
   <input type="hidden" name="mday" value="substring($sdate,7,2)"/>
   <input type="hidden" name="mon"  value="substring($sdate,5,2)"/>
   <input type="hidden" name="year" value="substring($sdate,1,4)"/>
   <input type="submit" name="submit" value="{$search}"/>
  </form>
 </div>
 <div id="smSearch">
  <form action="{server/cgi-url}/keyword.cgi">
   <input type="hidden" name="doc-url" value="{server/doc-url}"/>
   <input type="hidden" name="format"  value="{$ext}"/>
   <input type="hidden" name="query"   value="{translate(query,',',' ')}"/>
   <xsl:call-template name="date-fields">
    <xsl:with-param name="date" select="$sdate"/>
   </xsl:call-template>
   <input type="submit" name="submit" value="{$jump}"/>
  </form>
 </div>

 <table width="100%" cellspacing="0" cellpadding="0" class="padded" id="searchOut">
  <tr class="thRow">
   <th align="left"><xsl:value-of select="$subject"/></th>
   <th align="left"><xsl:value-of select="$author"/></th>
   <th align="left"><xsl:value-of select="$date"/> (UTC)</th></tr>
  <xsl:apply-templates select="row"/>
 </table>

 <br/>

</xsl:template>

</xsl:stylesheet>
