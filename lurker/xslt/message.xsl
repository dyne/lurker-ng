<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/1999/xhtml"
    version="1.0">

<xsl:template match="a" mode="snippet"><img alt="." src="../imgs/a.png"/></xsl:template>
<xsl:template match="b" mode="snippet"><img alt="|" src="../imgs/b.png"/></xsl:template>
<xsl:template match="c" mode="snippet"><img alt="-" src="../imgs/c.png"/></xsl:template>
<xsl:template match="d" mode="snippet"><img alt="\" src="../imgs/d.png"/></xsl:template>
<xsl:template match="e" mode="snippet"><img alt="+" src="../imgs/e.png"/></xsl:template>

<xsl:template match="f" mode="snippet">
 <a href="../message/{summary/mid}.{$ext}">
  <xsl:if test="@selected">
   <img
     title="{summary/time}: '{summary/email/@name}' {summary/email/@address}"
     alt="M" border="0" src="../imgs/f.png" class="selected"/>
  </xsl:if>
  <xsl:if test="not(@selected)">
   <img
     title="{summary/time}: '{summary/email/@name}' {summary/email/@address}"
     alt="m" border="0" src="../imgs/f.png"/>
  </xsl:if>
 </a>
</xsl:template>

<xsl:template match="g" mode="snippet">
 <a href="../message/{summary/mid}.{$ext}">
  <xsl:if test="@selected">
   <img
     title="{summary/time}: '{summary/email/@name}' {summary/email/@address}"
     alt="M" border="0" src="../imgs/g.png" class="selected"/>
  </xsl:if>
  <xsl:if test="not(@selected)">
   <img
     title="{summary/time}: '{summary/email/@name}' {summary/email/@address}"
     alt="m" border="0" src="../imgs/g.png"/>
  </xsl:if>
 </a>
</xsl:template>

<xsl:template match="h" mode="snippet">
 <a href="../message/{summary/mid}.{$ext}">
  <xsl:if test="@selected">
   <img
     title="{summary/time}: '{summary/email/@name}' {summary/email/@address}"
     alt="M" border="0" src="../imgs/h.png" class="selected"/>
  </xsl:if>
  <xsl:if test="not(@selected)">
   <img
     title="{summary/time}: '{summary/email/@name}' {summary/email/@address}"
     alt="m" border="0" src="../imgs/h.png"/>
  </xsl:if>
 </a>
</xsl:template>

<xsl:template match="i" mode="snippet">
 <a href="../message/{summary/mid}.{$ext}">
  <xsl:if test="@selected">
   <img
     title="{summary/time}: '{summary/email/@name}' {summary/email/@address}"
     alt="M" border="0" src="../imgs/i.png" class="selected"/>
  </xsl:if>
  <xsl:if test="not(@selected)">
   <img
     title="{summary/time}: '{summary/email/@name}' {summary/email/@address}"
     alt="m" border="0" src="../imgs/i.png"/>
  </xsl:if>
 </a>
</xsl:template>

<xsl:template match="j" mode="snippet">
 <a href="../message/{summary/mid}.{$ext}">
  <xsl:if test="@selected">
   <img
     title="{summary/time}: '{summary/email/@name}' {summary/email/@address}"
     alt="M" border="0" src="../imgs/j.png" class="selected"/>
  </xsl:if>
  <xsl:if test="not(@selected)">
   <img
     title="{summary/time}: '{summary/email/@name}' {summary/email/@address}"
     alt="m" border="0" src="../imgs/j.png"/>
  </xsl:if>
 </a>
</xsl:template>

<xsl:template match="k" mode="snippet">
 <a href="../message/{summary/mid}.{$ext}">
  <xsl:if test="@selected">
   <img
     title="{summary/time}: '{summary/email/@name}' {summary/email/@address}"
     alt="M" border="0" src="../imgs/k.png" class="selected"/>
  </xsl:if>
  <xsl:if test="not(@selected)">
   <img
     title="{summary/time}: '{summary/email/@name}' {summary/email/@address}"
     alt="m" border="0" src="../imgs/k.png"/>
  </xsl:if>
 </a>
</xsl:template>

<xsl:template match="br" mode="snippet"><br/></xsl:template>

<xsl:template match="list" mode="offset">
 <ul/>
 <a href="../mindex/{id}@{string(floor(number(offset) div 20)*20)}.{$ext}">
  <xsl:value-of select="email/@name"/> - message #
  <xsl:value-of select="offset"/>
 </a>
</xsl:template>

<xsl:template match="message" mode="title">
 <xsl:apply-templates select="server" mode="title"/> -
 <xsl:value-of select="list/email/@name"/> -
 <xsl:value-of select="subject"/>
</xsl:template>

<xsl:template match="message" mode="body">
 <p/><xsl:apply-templates select="server" mode="header"/>
 <p/><h2><a href="../thread/{thread}.{$ext}">Thread <xsl:value-of select="subject"/></a></h2>
 
 <p/><xsl:apply-templates select="threading/snippet" mode="snippet"/>

 <p/>
 <table>
  <col width="1*"/>
  <col width="100%"/>
  <tr>
   <th valign="top" align="left">Author:</th>
   <td><xsl:apply-templates mode="list" select="email"/></td>
  </tr>
  <tr><th valign="top" align="left">Date:</th><td><xsl:value-of select="time"/></td></tr>

  <tr><th valign="top" align="left">To:</th><td><xsl:apply-templates mode="list" select="to"/></td></tr>
  <xsl:if test="/message/cc">
   <tr><th valign="top" align="left">CC:</th><td><xsl:apply-templates mode="list" select="cc"/></td></tr>
  </xsl:if>
  
  <xsl:if test="threading/inreplyto">
   <tr><th valign="top" align="left">In-Reply-To:</th><td><xsl:apply-templates mode="list" select="threading/inreplyto/summary"/></td></tr>
  </xsl:if>
  <xsl:if test="threading/replies">
   <tr><th valign="top" align="left">Follow-Ups:</th><td><xsl:apply-templates mode="list" select="threading/replies/summary"/></td></tr>
  </xsl:if>
  <xsl:if test="threading/prev">
   <tr><th valign="top" align="left">Previous in Thread</th><td><xsl:apply-templates mode="list" select="threading/prev/summary"/></td></tr>
  </xsl:if>
  <xsl:if test="threading/next">
   <tr><th valign="top" align="left">Next in Thread</th><td><xsl:apply-templates mode="list" select="threading/next/summary"/></td></tr>
  </xsl:if>
 </table>
 
 <hr/>
 <p/>Appears in the <a href="../mbox/{mid}.txt">mailbox</a> of these mailing lists:
 <ul>
  <xsl:apply-templates select="list" mode="offset"/>
 </ul>

 <hr/>
 <blockquote>
  <xsl:apply-templates select="mime"/>
 </blockquote>
</xsl:template>

</xsl:stylesheet>
