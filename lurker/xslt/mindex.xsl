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
  <xsl:if test="prev">
   <a href="{list/id}@{prev}.{$ext}"><img border="0" src="../imgs/prev.png"/></a>
  </xsl:if>
  <xsl:if test="not(prev)">
   <img src="../imgs/a.png"/>
  </xsl:if>
  <xsl:if test="next">
   <a href="{list/id}@{next}.{$ext}"><img border="0" src="../imgs/next.png"/></a>
  </xsl:if>
  <xsl:if test="not(next)">
   <img src="../imgs/a.png"/>
  </xsl:if>

  <xsl:text disable-output-escaping="yes">&amp;nbsp;&amp;nbsp;&amp;nbsp;</xsl:text>

  <xsl:apply-templates select="list/email"/> - Messages
  <xsl:value-of select="offset"/>-<xsl:value-of select="string(number(offset)+count(summary)-1)"/>
 </h2>
 
 <p/>
 <table width="100%">
    <tr><th align="left">Subject</th>
        <th align="left">Author</th>
        <th align="left">Timestamp</th></tr>
    <xsl:apply-templates select="summary"/>
 </table>
</xsl:template>

</xsl:stylesheet>
