<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/1999/xhtml"
    version="1.0">

<!-- Format email address -->

<xsl:template match="email[@address]">
 <a href="mailto:{@address}">
  <xsl:if test="@name">&quot;<xsl:value-of select="@name"/>&quot;</xsl:if>
  &lt;<xsl:value-of select="@address"/>&gt;
 </a>
</xsl:template>

<xsl:template match="email">
 <xsl:if test="not(@name)">
  Anonymous
 </xsl:if>
 <xsl:value-of select="@name"/>
</xsl:template>

<xsl:template match="email[@address]" mode="list">
  <xsl:if test="not(position()=1)"><xsl:text>, </xsl:text></xsl:if>
  <xsl:apply-templates select="."/>
</xsl:template>

<!-- Format email message contents -->

<xsl:template match="br"><br/></xsl:template>

<xsl:template match="mime">
 <xsl:if test="not(position()=1)"><hr/></xsl:if>
 <div align="right">
  <xsl:if test="@name">
   <a href="../attach/{@id}@{/message/mid}@{@name}">
    <xsl:value-of select="@name"/> (<xsl:value-of select="@type"/>)
   </a>
   </xsl:if>
   <xsl:if test="not(@name)">
    <a href="../attach/{@id}@{/message/mid}.attach">
     (<xsl:value-of select="@type"/>)
    </a>
   </xsl:if>
  </div>
 <xsl:apply-templates/>
</xsl:template>

<!-- Format summary lists -->

<xsl:template match="a"><img src="i/a.png"/></xsl:template>
<xsl:template match="b"><img src="i/b.png"/></xsl:template>
<xsl:template match="c"><img src="i/c.png"/></xsl:template>
<xsl:template match="d"><img src="i/d.png"/></xsl:template>
<xsl:template match="e"><img src="i/e.png"/></xsl:template>
<xsl:template match="f"><a href="../message/{../../mid}.{$ext}"><img border="0" src="i/f.png"/></a></xsl:template>
<xsl:template match="g"><a href="../message/{../../mid}.{$ext}"><img border="0" src="i/g.png"/></a></xsl:template>
<xsl:template match="h"><a href="../message/{../../mid}.{$ext}"><img border="0" src="i/h.png"/></a></xsl:template>
<xsl:template match="i"><a href="../message/{../../mid}.{$ext}"><img border="0" src="i/i.png"/></a></xsl:template>
<xsl:template match="j"><a href="../message/{../../mid}.{$ext}"><img border="0" src="i/j.png"/></a></xsl:template>
<xsl:template match="k"><a href="../message/{../../mid}.{$ext}"><img border="0" src="i/k.png"/></a></xsl:template>

<xsl:template match="summary">
 <tr>
  <td nowrap="1">
   <xsl:if test="draw"><xsl:apply-templates select="draw"/></xsl:if>
   <xsl:if test="not(draw)">
    <a href="../message/{mid}.{$ext}"><xsl:value-of select="subject"/></a>
   </xsl:if>
  </td>
  <td><xsl:apply-templates select="email"/></td>
  <td><xsl:value-of select="time"/></td>
 </tr>
</xsl:template>

<xsl:template match="summary" mode="list">
  <xsl:if test="not(position()=1)"><xsl:text>, </xsl:text></xsl:if>
  <a href="{mid}.{$ext}">
    <xsl:choose>
      <xsl:when test="email/@name"><xsl:value-of select="email/@name"/></xsl:when>
      <xsl:when test="email/@address"><xsl:value-of select="email/@address"/></xsl:when>
      <xsl:otherwise>SomeOne</xsl:otherwise>
    </xsl:choose>'s post on
    <xsl:value-of select="time"/>
  </a>
</xsl:template>

<!-- Format server information segments -->

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
