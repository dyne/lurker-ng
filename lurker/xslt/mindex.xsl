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
   <a href="{list/id}@0.{$ext}"><img border="0" alt="&lt;&lt;=" src="../imgs/first.png"/></a>
   <a href="{list/id}@{prev}.{$ext}"><img border="0" alt="&lt;-" src="../imgs/prev.png"/></a>
  </xsl:if>
  <xsl:if test="not(prev)">
   <img alt=" " src="../imgs/a.png"/>
   <img alt=" " src="../imgs/a.png"/>
  </xsl:if>
  <xsl:if test="next">
   <a href="{list/id}@{next}.{$ext}"><img border="0" alt="-&gt;" src="../imgs/next.png"/></a>
   <a href="../lurker-jump.cgi?list={list/id}&amp;year=2038&amp;format={$ext}"><img border="0" alt="=&gt;&gt;" src="../imgs/last.png"/></a>
  </xsl:if>
  <xsl:if test="not(next)">
   <img alt=" " src="../imgs/a.png"/>
   <img alt=" " src="../imgs/a.png"/>
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
