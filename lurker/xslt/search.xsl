<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/1999/xhtml"
    version="1.0">

<xsl:template match="search" mode="title">
 <xsl:value-of select="$search"/>: <xsl:value-of select="query"/>
</xsl:template>

<xsl:template match="search" mode="body">
 <p/>
 <h2>
  <xsl:if test="prev">
   <a href="{prev}%20{queryurl}.{$ext}"><img border="0" src="../imgs/prev.png"/></a>
  </xsl:if>
  <xsl:if test="not(prev)">
   <img src="../imgs/a.png"/>
  </xsl:if>
  <xsl:if test="next">
   <a href="{next}%20{queryurl}.{$ext}"><img border="0" src="../imgs/next.png"/></a>
  </xsl:if>
  <xsl:if test="not(next)">
   <img src="../imgs/a.png"/>
  </xsl:if>

  <xsl:text disable-output-escaping="yes">&amp;nbsp;&amp;nbsp;&amp;nbsp;</xsl:text>
  
  <xsl:value-of select="$search"/> - <xsl:value-of select="$mess"/>
  <xsl:text> </xsl:text>
  <xsl:value-of select="offset"/>-<xsl:value-of select="string(number(offset)+count(summary)-1)"/>
  / ~<xsl:value-of select="hits"/>
 </h2>
 
 <p/>
 <form action="../lurker-search.cgi">
  <input type="hidden" name="format" value="{$ext}"/>
  <input type="text"   name="query"  value="{query}" size="50"/>
  <input type="submit" name="submit" value="{$search}!"/>
 </form> 

 <p/>
 <table width="100%" cellspacing="0" cellpadding="0" id="padded">
  <tr class="thRow">
   <th align="left"><xsl:value-of select="$subject"/></th>
   <th align="left"><xsl:value-of select="$author"/></th>
   <th align="left"><xsl:value-of select="$date"/></th></tr>
  <xsl:apply-templates select="summary"/>
 </table>
</xsl:template>

</xsl:stylesheet>
