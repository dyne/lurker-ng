<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    version="1.0">

<xsl:template match="search" mode="title">
 <xsl:value-of select="$search"/>: <xsl:value-of select="query"/>
</xsl:template>

<xsl:template match="search" mode="body">
 <h2>
  <xsl:choose>
   <xsl:when test="next">
    <a href="{next/summary/id}@{query}.{$ext}"><img alt="-&gt;" src="../imgs/next.png"/></a>
   </xsl:when>
   <xsl:otherwise>
    <img src="../imgs/a.png" alt="..."/>
   </xsl:otherwise>
  </xsl:choose>
 
  <xsl:text>&#160;&#160;&#160;</xsl:text>
  
  <xsl:value-of select="$search"/>
 </h2>
 
 <div id="smSearch">
  <form action="{server/cgi-url}/keyword.cgi">
   <input type="hidden" name="doc-url" value="{server/doc-url}"/>
   <input type="hidden" name="format"  value="{$ext}"/>
   <input type="text"   name="query"   value="{translate(query,',',' ')}" class="longtext"/>
   <xsl:call-template name="date-fields">
    <xsl:with-param name="date" select="$jump-date"/>
   </xsl:call-template>
   <input type="submit" name="submit" value="{$search}!"/>
  </form>
 </div>

 <table width="100%" cellspacing="0" cellpadding="0" class="padded" id="searchOut">
  <tr class="thRow">
   <th align="left"><xsl:value-of select="$subject"/></th>
   <th align="left"><xsl:value-of select="$author"/></th>
   <th align="left"><xsl:value-of select="$date"/></th></tr>
  <xsl:apply-templates select="row"/>
 </table>

 <br/>

</xsl:template>

</xsl:stylesheet>
