<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<xsl:import href="english.xsl"/>

<xsl:variable name="lurker-url" select="'http://lurker.sourceforge.net/'"/>
<xsl:variable name="last-date" select="'20380101.000000.00000000'"/>
<xsl:variable name="jump-date" select="'20040101.000000.00000000'"/>

<!-- Output control -->
<xsl:variable name="ext" select="'html'"/>
<xsl:output method="html" indent="no" encoding="UTF-8"
            doctype-system="http://www.w3.org/TR/html4/strict.dtd"
            doctype-public="-//W3C//DTD HTML 4.0 Transitional//EN"/>


<!-- Email formatting -->
<xsl:template match="email" mode="email-name">
 <xsl:choose>
  <xsl:when test="@name"><xsl:value-of select="@name"/></xsl:when>
  <xsl:when test="@address"><xsl:value-of select="@address"/></xsl:when>
  <xsl:otherwise><xsl:value-of select="$unknown-address"/></xsl:otherwise>
 </xsl:choose>
</xsl:template>
<xsl:template match="email[@address]" mode="email-link">
 <a href="mailto:{@address}">
  <xsl:apply-templates mode="email-name" select="."/>
 </a>
</xsl:template>
<xsl:template match="email" mode="email-link">
 <xsl:apply-templates mode="email-name" select="."/>
</xsl:template>
<xsl:template match="email" mode="email-list">
 <xsl:if test="position()!=1">, </xsl:if>
 <xsl:apply-templates mode="email-link" select="."/>
</xsl:template>

<!-- Date formatting -->
<xsl:template match="summary" mode="utc-date">
 <xsl:value-of select="substring(id,1,4)"/>
 <xsl:text>-</xsl:text>
 <xsl:value-of select="substring(id,5,2)"/>
 <xsl:text>-</xsl:text>  
 <xsl:value-of select="substring(id,7,2)"/>
 <xsl:text>&#160;</xsl:text>
 <xsl:value-of select="substring(id,10,2)"/>
 <xsl:text>:</xsl:text>
 <xsl:value-of select="substring(id,12,2)"/>
</xsl:template>
<xsl:template match="summary" mode="text-date">
 <script type="text/javascript"><xsl:comment>
textdate(<xsl:value-of select="timestamp"/>);//</xsl:comment></script>
 <noscript><xsl:apply-templates mode="utc-date" select="."/></noscript>
</xsl:template>
<xsl:template match="summary" mode="timezone">
 <script type="text/javascript"><xsl:comment>
timezone(<xsl:value-of select="timestamp"/>);//</xsl:comment></script>
 <noscript>UTC</noscript>
</xsl:template>

<!-- Summary formatting -->
<xsl:template match="summary" mode="post-description-text">
 <xsl:apply-templates mode="email-name" select="email"/>
 <xsl:value-of select="$posted-at"/>
 <xsl:apply-templates mode="text-date" select="."/>
</xsl:template>
<xsl:template match="summary" mode="post-description-link">
 <a href="../message/{id}.{$ext}">
  <xsl:apply-templates mode="post-description-text" select="."/>
 </a>
</xsl:template>
<xsl:template match="summary" mode="post-description-list">
 <xsl:if test="position()!=1">, </xsl:if>
 <xsl:apply-templates mode="post-description-link" select="."/>
</xsl:template>
<xsl:template match="summary" mode="post-description-title">
 <xsl:attribute name="title">
  <xsl:apply-templates mode="email-name" select="email"/>
  <xsl:value-of select="$posted-at"/>
  <xsl:apply-templates mode="utc-date" select="."/>
 </xsl:attribute>
 <xsl:attribute name="onMouseOver">
  <xsl:text>titledate(this,</xsl:text>
  <xsl:value-of select="timestamp"/>
  <xsl:text>);</xsl:text>
 </xsl:attribute>
</xsl:template>


<!-- Tree formatting -->
<xsl:template name="tree-link">
 <xsl:variable name="id" select="@id"/>
 <a href="../message/{$id}.{$ext}">
  <xsl:element name="img">
   <xsl:attribute name="src">
    <xsl:text>../imgs/</xsl:text>
    <xsl:value-of select="local-name(.)"/>
    <xsl:text>.png</xsl:text>
   </xsl:attribute>
   <xsl:apply-templates mode="post-description-title" select="../../summary[id=$id]"/>
   <xsl:if test="@selected">
    <xsl:attribute name="class">selected</xsl:attribute>
   </xsl:if>
   <xsl:attribute name="alt">M</xsl:attribute>
  </xsl:element>
 </a>
</xsl:template>
<xsl:template mode="tree" match="a"><img alt="." src="../imgs/a.png"/></xsl:template>
<xsl:template mode="tree" match="b"><img alt="|" src="../imgs/b.png"/></xsl:template>
<xsl:template mode="tree" match="c"><img alt="-" src="../imgs/c.png"/></xsl:template>
<xsl:template mode="tree" match="d"><img alt="\" src="../imgs/d.png"/></xsl:template>
<xsl:template mode="tree" match="e"><img alt="+" src="../imgs/e.png"/></xsl:template>
<xsl:template mode="tree" match="f"><xsl:call-template name="tree-link"/></xsl:template>
<xsl:template mode="tree" match="g"><xsl:call-template name="tree-link"/></xsl:template>
<xsl:template mode="tree" match="h"><xsl:call-template name="tree-link"/></xsl:template>
<xsl:template mode="tree" match="i"><xsl:call-template name="tree-link"/></xsl:template>
<xsl:template mode="tree" match="j"><xsl:call-template name="tree-link"/></xsl:template>  
<xsl:template mode="tree" match="k"><xsl:call-template name="tree-link"/></xsl:template>  


<!-- Date fields -->
<xsl:template name="option-range">
 <xsl:param name="start"/>
 <xsl:param name="last"/>
 <xsl:param name="select"/>
 <xsl:element name="option">
  <xsl:attribute name="value"><xsl:value-of select="$start"/></xsl:attribute>
  <xsl:if test="number($start) = number($select)">
   <xsl:attribute name="selected">SELECTED</xsl:attribute>
  </xsl:if>
  <xsl:value-of select="$start"/>
 </xsl:element>
 <xsl:if test="$start &lt; $last">
  <xsl:call-template name="option-range">
   <xsl:with-param name="start"> <xsl:value-of select="$start+1"/></xsl:with-param>
   <xsl:with-param name="last"> <xsl:value-of select="$last"/></xsl:with-param>
   <xsl:with-param name="select"><xsl:value-of select="$select"/> </xsl:with-param>
  </xsl:call-template>
 </xsl:if>
</xsl:template>
<xsl:template name="hour-range">
 <xsl:param name="start"/>
 <xsl:param name="select"/>
 <xsl:element name="option">
  <xsl:attribute name="value"><xsl:value-of select="$start"/></xsl:attribute>
  <xsl:if test="number($start) = number($select)">
   <xsl:attribute name="selected">SELECTED</xsl:attribute>
  </xsl:if>
  <xsl:value-of select="$start"/>:00
 </xsl:element>
 <xsl:if test="$start &lt; 23">
  <xsl:call-template name="hour-range">
   <xsl:with-param name="start"> <xsl:value-of select="$start+1"/></xsl:with-param>
   <xsl:with-param name="select"><xsl:value-of select="$select"/> </xsl:with-param>
  </xsl:call-template>
 </xsl:if>
</xsl:template>
<xsl:template name="date-fields">
 <xsl:param name="date"/>
 <input type="hidden" name="utc" value="0"/>
 <input type="hidden" name="sec" value="{substring($date,14,2)}"/>
 <input type="hidden" name="min" value="{substring($date,12,2)}"/>
 <select name="hour">
  <xsl:call-template name="hour-range">
   <xsl:with-param name="start">0</xsl:with-param>
   <xsl:with-param name="select" select="substring($date,10,2)"/>
  </xsl:call-template>
 </select>
 <select name="mday">
  <xsl:call-template name="option-range">
   <xsl:with-param name="start">1</xsl:with-param>
   <xsl:with-param name="last">31</xsl:with-param>
   <xsl:with-param name="select" select="substring($date,7,2)"/>
  </xsl:call-template>
 </select>
 <select name="mon">
  <xsl:element name="option">
   <xsl:attribute name="value">1</xsl:attribute>
   <xsl:if test="substring($date,5,2) = 1"><xsl:attribute name="selected">SELECTED</xsl:attribute></xsl:if>
   <xsl:value-of select="$jan"/>
  </xsl:element>
  <xsl:element name="option">
   <xsl:attribute name="value">2</xsl:attribute>
   <xsl:if test="substring($date,5,2) = 2"><xsl:attribute name="selected">SELECTED</xsl:attribute></xsl:if>
   <xsl:value-of select="$feb"/>
  </xsl:element>
  <xsl:element name="option">
   <xsl:attribute name="value">3</xsl:attribute>
   <xsl:if test="substring($date,5,2) = 3"><xsl:attribute name="selected">SELECTED</xsl:attribute></xsl:if>
   <xsl:value-of select="$mar"/>
  </xsl:element>
  <xsl:element name="option">
   <xsl:attribute name="value">4</xsl:attribute>
   <xsl:if test="substring($date,5,2) = 4"><xsl:attribute name="selected">SELECTED</xsl:attribute></xsl:if>
   <xsl:value-of select="$apr"/>
  </xsl:element>
  <xsl:element name="option">
   <xsl:attribute name="value">5</xsl:attribute>
   <xsl:if test="substring($date,5,2) = 5"><xsl:attribute name="selected">SELECTED</xsl:attribute></xsl:if>
   <xsl:value-of select="$may"/>
  </xsl:element>
  <xsl:element name="option">
   <xsl:attribute name="value">6</xsl:attribute>
   <xsl:if test="substring($date,5,2) = 6"><xsl:attribute name="selected">SELECTED</xsl:attribute></xsl:if>
   <xsl:value-of select="$jun"/>
  </xsl:element>
  <xsl:element name="option">
   <xsl:attribute name="value">7</xsl:attribute>
   <xsl:if test="substring($date,5,2) = 7"><xsl:attribute name="selected">SELECTED</xsl:attribute></xsl:if>
   <xsl:value-of select="$jul"/>
  </xsl:element>
  <xsl:element name="option">
   <xsl:attribute name="value">8</xsl:attribute>
   <xsl:if test="substring($date,5,2) = 8"><xsl:attribute name="selected">SELECTED</xsl:attribute></xsl:if>
   <xsl:value-of select="$aug"/>
  </xsl:element>
  <xsl:element name="option">
   <xsl:attribute name="value">9</xsl:attribute>
   <xsl:if test="substring($date,5,2) = 9"><xsl:attribute name="selected">SELECTED</xsl:attribute></xsl:if>
   <xsl:value-of select="$sep"/>
  </xsl:element>
  <xsl:element name="option">
   <xsl:attribute name="value">10</xsl:attribute>
   <xsl:if test="substring($date,5,2) = 10"><xsl:attribute name="selected">SELECTED</xsl:attribute></xsl:if>
   <xsl:value-of select="$oct"/>
  </xsl:element>
  <xsl:element name="option">
   <xsl:attribute name="value">11</xsl:attribute>
   <xsl:if test="substring($date,5,2) = 11"><xsl:attribute name="selected">SELECTED</xsl:attribute></xsl:if>
   <xsl:value-of select="$nov"/>
  </xsl:element>
  <xsl:element name="option">
   <xsl:attribute name="value">12</xsl:attribute>
   <xsl:if test="substring($date,5,2) = 12"><xsl:attribute name="selected">SELECTED</xsl:attribute></xsl:if>
  <xsl:value-of select="$dec"/>
  </xsl:element>
 </select>
 <select name="year">
  <xsl:call-template name="option-range">
   <xsl:with-param name="start">1990</xsl:with-param>
   <xsl:with-param name="last">2004</xsl:with-param>
   <xsl:with-param name="select" select="substring($date,1,4)"/>
  </xsl:call-template>
 </select>
</xsl:template> 


<!-- Common components -->
<xsl:template name="lurker-signature">
 <table class="external">
  <tr>
   <td class="mini">
    <b><a href="{$lurker-url}">Lurker</a></b>
    (<xsl:value-of select="$version"/> <xsl:value-of select="server/version"/>)
   </td>
   <td class="mini" align="right">
    <xsl:value-of select="$admin-by"/>
    <xsl:apply-templates mode="email-link" select="server/email"/>
   </td>
  </tr>
 </table>
</xsl:template>


<!-- Common links -->
<xsl:template match="server" mode="splash-link">
 <a href="../splash/index.{$ext}"><xsl:value-of select="archive"/></a>
</xsl:template>
<xsl:template match="list" mode="list-link">
 <a href="../list/{id}.{$ext}">
  <xsl:apply-templates mode="email-name" select="email"/>
 </a>
</xsl:template>
<xsl:template match="summary" mode="thread-link">
 <a href="../thread/{id}.{$ext}"><xsl:value-of select="subject"/></a>
</xsl:template>


</xsl:stylesheet>
