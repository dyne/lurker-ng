<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/1999/xhtml"
    version="1.0">

<xsl:template match="mindex" mode="title">
 <xsl:value-of select="list/email/@name"/> -
 Messages <xsl:value-of select="offset"/>-<xsl:value-of select="string(number(offset)+count(summary)-1)"/>
</xsl:template>

<xsl:template match="mindex" mode="body">
 <p/>
 <h2>
  <xsl:apply-templates select="list/email"/> - Messages
  <xsl:value-of select="offset"/>-<xsl:value-of select="string(number(offset)+count(summary)-1)"/>
 </h2>
 <p/>
 <h2>
  <xsl:if test="prev">
   <a href="{list/id}@{prev}.{$ext}">(&lt;&lt; prev)</a>
  </xsl:if>
  <xsl:if test="next">
   <a href="{list/id}@{next}.{$ext}">(next &gt;&gt;)</a>
  </xsl:if>
 </h2>

 <table border="1">
    <tr><th align="left">Subject</th>
        <th align="left">Author</th>
        <th align="left">Timestamp</th></tr>
    <xsl:apply-templates select="summary"/>
 </table>
</xsl:template>

</xsl:stylesheet>
