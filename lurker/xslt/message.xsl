<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/1999/xhtml"
    version="1.0">

<xsl:template name="msg-snippet">
 <a href="../message/{summary/mid}.{$ext}">
  <xsl:if test="@selected">
   <img
     title="{summary/time}: '{summary/email/@name}' {summary/email/@address}"
     alt="M" border="0" src="../imgs/{local-name(.)}.png" class="selected"/>
  </xsl:if>
  <xsl:if test="not(@selected)">
   <img
     title="{summary/time}: '{summary/email/@name}' {summary/email/@address}"
     alt="m" border="0" src="../imgs/{local-name(.)}.png"/>
  </xsl:if>
 </a>
</xsl:template>

<xsl:template name="header-fields">
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
 </table>
</xsl:template>

<xsl:template name="navigate-fields">
 <div align="right">
  <xsl:if test="threading/prev">
   <xsl:apply-templates select="threading/prev" mode="snippet"/>
  </xsl:if>
  <xsl:if test="not(threading/prev)">
   <img src="../imgs/a.png"/>
  </xsl:if>
  <xsl:if test="reply">
   <a href="mailto:{reply}">(reply)</a>
  </xsl:if>
  <xsl:if test="not(reply)">
   (reply)
  </xsl:if>
  <xsl:if test="threading/next">
   <xsl:apply-templates select="threading/next" mode="snippet"/>
  </xsl:if>
  <xsl:if test="not(threading/next)">
   <img src="../imgs/a.png"/>
  </xsl:if>
 </div>
 <hr/>
 <xsl:apply-templates select="threading/snippet" mode="snippet"/>
</xsl:template>

<xsl:template match="a" mode="snippet"><img alt="." src="../imgs/a.png"/></xsl:template>
<xsl:template match="b" mode="snippet"><img alt="|" src="../imgs/b.png"/></xsl:template>
<xsl:template match="c" mode="snippet"><img alt="-" src="../imgs/c.png"/></xsl:template>
<xsl:template match="d" mode="snippet"><img alt="\" src="../imgs/d.png"/></xsl:template>
<xsl:template match="e" mode="snippet"><img alt="+" src="../imgs/e.png"/></xsl:template>

<xsl:template match="f" mode="snippet"><xsl:call-template name="msg-snippet"/></xsl:template>
<xsl:template match="g" mode="snippet"><xsl:call-template name="msg-snippet"/></xsl:template>
<xsl:template match="h" mode="snippet"><xsl:call-template name="msg-snippet"/></xsl:template>
<xsl:template match="i" mode="snippet"><xsl:call-template name="msg-snippet"/></xsl:template>
<xsl:template match="j" mode="snippet"><xsl:call-template name="msg-snippet"/></xsl:template>
<xsl:template match="k" mode="snippet"><xsl:call-template name="msg-snippet"/></xsl:template>

<xsl:template match="prev" mode="snippet"><xsl:call-template name="msg-snippet"/></xsl:template>
<xsl:template match="next" mode="snippet"><xsl:call-template name="msg-snippet"/></xsl:template>

<xsl:template match="br" mode="snippet"><br/></xsl:template>

<xsl:template match="list" mode="offset">
 <ul/>
 <a href="../mindex/{id}@{string(floor(number(offset) div 20)*20)}.{$ext}#{id}">
  <xsl:value-of select="email/@name"/> - message 
  #<xsl:value-of select="offset"/>
 </a>
</xsl:template>

<xsl:template match="message" mode="title">
 <xsl:value-of select="list/email/@name"/> -
 <xsl:value-of select="subject"/>
</xsl:template>

<xsl:template match="message" mode="body">
 <p/><h2><a href="../thread/{thread}.{$ext}#{id}"><xsl:value-of select="subject"/></a></h2>

 <p/>
 <xsl:if test="threading/snippet/@cols &gt; 8">
  <xsl:call-template name="navigate-fields"/>
  <hr/>
  <xsl:call-template name="header-fields"/>
 </xsl:if>
 <xsl:if test="not(threading/snippet/@cols &gt; 8)">
  <table>
   <tr>
    <td width="100%" valign="top" nowrap="1"><xsl:call-template name="header-fields"/></td>
    <td valign="top" nowrap="1"><xsl:call-template name="navigate-fields"/></td>
   </tr>
  </table>
 </xsl:if>

 <p/>Appears in the <a href="../mbox/{mid}.txt">mailbox</a> of
 
 <xsl:if test="count(list) = 1">
  <a href="../mindex/{list/id}@{string(floor(number(list/offset) div 20)*20)}.{$ext}#{id}">
   <xsl:value-of select="list/email/@name"/> - message 
   #<xsl:value-of select="list/offset"/>
  </a>
 </xsl:if>
 
 <xsl:if test="count(list) != 1">
  these mailing lists:
  <ul>
   <xsl:apply-templates select="list" mode="offset"/>
  </ul>
 </xsl:if>

 <p/><hr/>
 <blockquote>
  <xsl:apply-templates select="mime"/>
 </blockquote>

</xsl:template>

</xsl:stylesheet>
