<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/1999/xhtml"
    version="1.0">

<xsl:template match="search" mode="title">
 Search: <xsl:value-of select="query"/>
</xsl:template>

<xsl:template match="search" mode="body">
 <p/><h2>Messages <xsl:value-of select="offset"/> / <xsl:value-of select="hits"/></h2>
 <p/>
 <h2>
  <xsl:if test="/search/prev">
   <a href="{/search/prev}%20{/search/queryurl}.html">(&lt;&lt; prev)</a>
  </xsl:if>
  <xsl:if test="/search/next">
   <a href="{/search/next}%20{/search/queryurl}.html">(next &gt;&gt;)</a>
  </xsl:if>
 </h2>
 
 <p/>
 <form action="../lurker-search.cgi">
  <input type="hidden" name="format" value="{$ext}"/>
  <input type="text"   name="query"  value="{/search/query}" size="50"/>
  <input type="submit" name="submit" value="Search!"/>
 </form> 

 <p/>
 <table border="1">
  <tr><th align="left">Subject</th>
      <th align="left">Author</th>
      <th align="left">Timestamp</th></tr>
  <xsl:apply-templates select="summary"/>
 </table>
</xsl:template>

</xsl:stylesheet>
