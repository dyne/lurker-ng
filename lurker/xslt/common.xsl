<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/1999/xhtml"
    version="1.0">


<!-- Truncate long strings -->

<xsl:template name="truncate">
 <xsl:param name="length"/>
 <xsl:param name="string"/>
 <xsl:choose>
  <xsl:when test="string-length($string) &gt; $length">
   <xsl:value-of select="substring($string,1,$length - 2)"/>
   <xsl:text> ...</xsl:text>
  </xsl:when>
  <xsl:otherwise>
   <xsl:value-of select="$string"/>
  </xsl:otherwise>
 </xsl:choose>
</xsl:template>


<!-- Format email address -->

<xsl:template name="email-simple">
 <xsl:choose>
  <xsl:when test="@name"><xsl:value-of select="@name"/></xsl:when>
  <xsl:when test="@address"><xsl:value-of select="@address"/></xsl:when>
  <xsl:otherwise><xsl:value-of select="$someone"/></xsl:otherwise>
 </xsl:choose>
</xsl:template>

<xsl:template name="email-cut">
 <xsl:call-template name="truncate">
  <xsl:with-param name="length">20</xsl:with-param>
  <xsl:with-param name="string"><xsl:call-template name="email-simple"/></xsl:with-param>
 </xsl:call-template>
</xsl:template>

<xsl:template match="email" mode="simple">
 <xsl:call-template name="email-simple"/>
</xsl:template>

<xsl:template match="email[@address]">
 <a href="mailto:{@address}">
  <xsl:call-template name="email-cut"/>
 </a>
</xsl:template>

<xsl:template match="email">
  <xsl:call-template name="email-cut"/>
</xsl:template>

<xsl:template match="email[@address]" mode="list">
 <xsl:if test="not(position()=1)"><xsl:text>, </xsl:text><br/></xsl:if>
 <a href="mailto:{@address}">
  <xsl:call-template name="email-simple"/>
 </a>
</xsl:template>

<xsl:template match="email" mode="list">
 <xsl:if test="not(position()=1)"><xsl:text>, </xsl:text><br/></xsl:if>
  <xsl:call-template name="email-simple"/>
</xsl:template>


<!-- Format summary lists -->

<xsl:template name="msg-thread">
 <a href="../message/{../../mid}.{$ext}">
  <xsl:element name="img">
   <xsl:attribute name="alt">M</xsl:attribute>
   <xsl:attribute name="border">0</xsl:attribute>
   
   <xsl:attribute name="src">
    <xsl:text>i/</xsl:text>
    <xsl:value-of select="local-name(.)"/>
    <xsl:text>.png</xsl:text>
   </xsl:attribute>
   
   <xsl:attribute name="class">
    <xsl:choose>
     <xsl:when test="@selected">selected</xsl:when>
     <xsl:when test="@drift">drift</xsl:when>
     <xsl:otherwise>normal</xsl:otherwise>
    </xsl:choose>
   </xsl:attribute>
  </xsl:element>
 </a>
</xsl:template>

<xsl:template match="a"><img alt="." src="i/a.png"/></xsl:template>
<xsl:template match="b"><img alt="|" src="i/b.png"/></xsl:template>
<xsl:template match="c"><img alt="-" src="i/c.png"/></xsl:template>
<xsl:template match="d"><img alt="\" src="i/d.png"/></xsl:template>
<xsl:template match="e"><img alt="+" src="i/e.png"/></xsl:template>
<xsl:template match="f"><xsl:call-template name="msg-thread"/></xsl:template>
<xsl:template match="g"><xsl:call-template name="msg-thread"/></xsl:template>
<xsl:template match="h"><xsl:call-template name="msg-thread"/></xsl:template>
<xsl:template match="i"><xsl:call-template name="msg-thread"/></xsl:template>
<xsl:template match="j"><xsl:call-template name="msg-thread"/></xsl:template>
<xsl:template match="k"><xsl:call-template name="msg-thread"/></xsl:template>

<xsl:template match="summary">
 <xsl:element name="tr">
  <xsl:if test="(position() mod 2) = 0">
   <xsl:attribute name="class">row1</xsl:attribute>
  </xsl:if>
  <xsl:if test="(position() mod 2) = 1">
   <xsl:attribute name="class">row2</xsl:attribute>
  </xsl:if>
  <td width="60%" nowrap="1">
   <a name="{id}"/>
   <xsl:if test="draw"><xsl:apply-templates select="draw"/></xsl:if>
   <xsl:if test="not(draw)">
    <a href="../thread/{thread}.{$ext}">
     <img border="0" src="../imgs/tree.png" alt="{$threading} "/>
     <xsl:text disable-output-escaping="yes">&amp;nbsp;</xsl:text>
    </a>
    <a href="../message/{mid}.{$ext}">
     <xsl:call-template name="truncate">
      <xsl:with-param name="length">50</xsl:with-param>
      <xsl:with-param name="string"><xsl:value-of select="subject"/></xsl:with-param>
     </xsl:call-template>
    </a>
    <xsl:text disable-output-escaping="yes">&amp;nbsp;&amp;nbsp;</xsl:text>
   </xsl:if>
  </td>
  <td width="25%" nowrap="1">
   <xsl:apply-templates select="email"/>
   <xsl:text disable-output-escaping="yes">&amp;nbsp;&amp;nbsp;</xsl:text>
  </td>
  <td width="15%" nowrap="1"><xsl:value-of select="time"/></td>
 </xsl:element>
</xsl:template>


<!-- Format list of threading summary lists -->

<xsl:template name="summary-post">
 <xsl:apply-templates select="email" mode="simple"/>
 <xsl:value-of select="$post"/>
 <xsl:text> </xsl:text>
 <xsl:value-of select="time"/>
</xsl:template>

<xsl:template match="summary" mode="post">
 <xsl:call-template name="summary-post"/>
</xsl:template>

<xsl:template match="summary" mode="list">
 <xsl:if test="not(position()=1)"><xsl:text>, </xsl:text><br/></xsl:if>
 <a href="{mid}.{$ext}">
  <xsl:call-template name="summary-post"/>
 </a>
</xsl:template>


<!-- Format server information segments -->

<xsl:template match="server" mode="title">
  Lurker@<xsl:value-of select="hostname"/> 
</xsl:template>

<xsl:template match="server" mode="header">
 <a href="../splash/index.{$ext}">
  Lurker@<xsl:value-of select="hostname"/>
 </a>
</xsl:template>

<xsl:template match="server" mode="footer">
 <xsl:copy-of select="$adminby"/> <xsl:apply-templates select="email"/>
</xsl:template>

</xsl:stylesheet>
