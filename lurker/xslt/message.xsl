<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/1999/xhtml"
    version="1.0">


<!-- Format messages in the snippet view -->

<xsl:template name="msg-snippet">
 <a href="../message/{summary/mid}.{$ext}">
  <xsl:element name="img">
   <xsl:attribute name="alt">M</xsl:attribute>
   <xsl:attribute name="border">0</xsl:attribute>
   
   <xsl:attribute name="title">
    <xsl:apply-templates select="summary" mode="post"/>
   </xsl:attribute>
   
   <xsl:attribute name="src">
    <xsl:text>../imgs/</xsl:text>
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


<!-- Format the drawing elements of a snippet -->

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


<!-- Format the navigation graphics -->

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


<!-- Format mailing list information -->

<xsl:template name="list-offset">
 <a href="../mindex/{id}@{string(floor(number(offset) div 20)*20)}.{$ext}#{../id}">
  <xsl:value-of select="email/@name"/> - message 
  #<xsl:value-of select="offset"/>
 </a>
</xsl:template>

<xsl:template match="list" mode="offset">
 <xsl:call-template name="list-offset"/>
</xsl:template>

<xsl:template match="list" mode="multiple">
 <li> <xsl:call-template name="list-offset"/></li>
</xsl:template>


<!-- Format headers for the message -->

<xsl:template name="header-fields">
 <table>
  <col width="1*"/>
  <col width="100%"/>
  <tr>
   <th valign="top" align="left">Author:</th>
   <td><xsl:apply-templates select="email" mode="list"/></td>
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


<!-- Format email message contents -->

<xsl:template match="mailto">
 <a class="mailto" href="mailto:{.}"><xsl:value-of select="."/></a>
</xsl:template>

<xsl:template match="url">
 <a class="url" href="{.}"><xsl:value-of select="."/></a>
</xsl:template>

<xsl:template match="quote">
 <i class="quote"><xsl:apply-templates/></i>
</xsl:template>

<xsl:template match="br"><br/></xsl:template>


<!-- Format the mime components of the email -->

<xsl:template match="mime">
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


<!-- The main formatting procedure for a message -->


<xsl:template match="message" mode="title">
 <xsl:value-of select="list/email/@name"/> -
 <xsl:value-of select="subject"/>
</xsl:template>

<xsl:template match="message" mode="body">
 <p/>
 <h2>
  <a href="../thread/{thread}.{$ext}#{id}">
   <xsl:value-of select="subject"/>
  </a>
 </h2>

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

 <xsl:if test="count(list) != 1"><p/></xsl:if>
 Appears in the <a href="../mbox/{mid}.txt">mailbox</a> of
 
 <xsl:if test="count(list) = 1">
  <xsl:apply-templates select="list" mode="offset"/>
 </xsl:if>
 <xsl:if test="count(list) != 1">
  these mailing lists:
  <ul><xsl:apply-templates select="list" mode="multiple"/></ul>
 </xsl:if>

 <p/><hr/>
 <blockquote>
  <xsl:apply-templates select="mime"/>
 </blockquote>
</xsl:template>

</xsl:stylesheet>
