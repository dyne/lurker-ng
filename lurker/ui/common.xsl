<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">


<!-- xsl:import href="lang.xsl"/ -->
<xsl:variable name="lang" select="'en'"/>

<xsl:variable name="tree-context" select="'Thread Context'"/>
<xsl:variable name="full-tree" select="'the complete thread tree sorted by date'"/>
<xsl:variable name="appears-in" select="'Message appears on the following lists:'"/>
<xsl:variable name="list-info" select="'List Info'"/>
<xsl:variable name="near-message" select="'Messages Nearby'"/>
<xsl:variable name="post-new" select="'Post New Thread'"/>
<xsl:variable name="author" select="'Author'"/>
<xsl:variable name="date" select="'Date'"/>
<xsl:variable name="to" select="'To'"/>
<xsl:variable name="cc" select="'CC'"/>
<xsl:variable name="old-topics" select="'Old-Topics'"/>
<xsl:variable name="new-topics" select="'New-Topics'"/>

<xsl:variable name="unknown-address" select="'Someone'"/>
<xsl:variable name="posted-at" select="' at '"/>
<xsl:variable name="admin-by" select="'Administrated by:'"/>

<xsl:variable name="lurker-url" select="'http://lurker.sourceforge.net/'"/>


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
<xsl:template match="summary" mode="date">
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


<!-- Summary formatting -->
<xsl:template match="summary" mode="post-description">
 <xsl:apply-templates mode="email-name" select="email"/>
 <xsl:value-of select="$posted-at"/>
 <xsl:apply-templates mode="date" select="."/>
</xsl:template>
<xsl:template match="summary" mode="post-description-link">
 <a href="../message/{id}">
  <xsl:apply-templates mode="post-description" select="."/>
 </a>
</xsl:template>
<xsl:template match="summary" mode="post-description-list">
 <xsl:if test="position()!=1">, </xsl:if>
 <xsl:apply-templates mode="post-description-link" select="."/>
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
   <xsl:attribute name="title">
    <xsl:apply-templates mode="post-description" select="../../summary[id=$id]"/>
   </xsl:attribute>
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
