<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"                    
    xmlns="http://www.w3.org/1999/xhtml"
    version="1.0">

<xsl:template match="email[@address]">
 <a href="mailto:{@address}">
  <xsl:if test="@name">&quot;<xsl:value-of select="@name"/>&quot;</xsl:if>
  &lt;<xsl:value-of select="@address"/>&gt;
 </a>
</xsl:template>

<xsl:template match="email">
 <xsl:if test="not(@name)">
  SomeOne
 </xsl:if>
 <xsl:value-of select="@name"/>
</xsl:template>

<xsl:template match="summary">
 <tr>
  <td><a href="../message/{id}.html"><xsl:value-of select="subject"/></a></td>
  <td><xsl:apply-templates select="email"/></td>
  <td><xsl:value-of select="time"/></td>
 </tr>
</xsl:template>

<xsl:template match="server" mode="title">
  Lurker@<xsl:value-of select="hostname"/> 
</xsl:template>

<xsl:template match="server" mode="header">
 <h1><a href="../splash/index.{$ext}">
  Lurker@<xsl:value-of select="hostname"/>
 </a></h1>
</xsl:template>

<xsl:template match="server" mode="footer">
 <xsl:copy-of select="$adminby"/><xsl:apply-templates select="email"/>
</xsl:template>

</xsl:stylesheet>
