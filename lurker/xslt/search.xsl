<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/1999/xhtml"
    version="1.0">

<xsl:template match="search" mode="title">
 Search: <xsl:value-of select="query"/>
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
  
  Search - Messages
  <xsl:value-of select="offset"/>-<xsl:value-of select="string(number(offset)+count(summary)-1)"/>
  / ~<xsl:value-of select="hits"/>
 </h2>
 
 <p/>
 <form action="../lurker-search.cgi">
  <input type="hidden" name="format" value="{$ext}"/>
  <input type="text"   name="query"  value="{query}" size="50"/>
  <input type="submit" name="submit" value="Search!"/>
 </form> 

 <p/>
 <table border="1" width="100%">
  <tr><th align="left">Subject</th>
      <th align="left">Author</th>
      <th align="left">Timestamp</th></tr>
  <xsl:apply-templates select="summary"/>
 </table>
</xsl:template>

</xsl:stylesheet>
