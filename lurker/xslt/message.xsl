<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/1999/xhtml"
    version="1.0">

<xsl:template match="message" mode="title">
 <xsl:apply-templates select="server" mode="title"/> -
 <xsl:value-of select="list/email/@name"/> -
 <xsl:value-of select="subject"/>
</xsl:template>

<xsl:template match="message" mode="body">
 <p/><xsl:apply-templates select="server" mode="header"/>
 <p/><h2>Thread <a href="../thread/{thread}.{$ext}"><xsl:value-of select="subject"/></a></h2>
 
 <p/>
 <table>
  <col width="1*"/>
  <col width="100%"/>
  <tr>
   <th align="left">Author:</th>
   <td>
    <xsl:choose>
     <xsl:when test="reply-to"><xsl:apply-templates mode="list" select="reply-to"/></xsl:when>
     <xsl:when test="from">    <xsl:apply-templates mode="list" select="from"/>    </xsl:when>
     <xsl:when test="sender">  <xsl:apply-templates mode="list" select="sender"/>  </xsl:when>
    </xsl:choose>
   </td>
  </tr>
  <tr><th align="left">To:</th><td><xsl:apply-templates mode="list" select="to"/></td></tr>
  <xsl:if test="/message/cc">
   <tr><th align="left">CC:</th><td><xsl:apply-templates mode="list" select="cc"/></td></tr>
  </xsl:if>
  <tr><th align="left">Date:</th><td><xsl:value-of select="time"/></td></tr>
  <xsl:if test="/message/inreplyto">
   <tr><th align="left">In-Reply-To:</th><td><xsl:apply-templates mode="list" select="inreplyto/summary"/></td></tr>
  </xsl:if>
  <xsl:if test="/message/replies">
   <tr><th align="left">Follow-Ups:</th><td><xsl:apply-templates mode="list" select="replies/summary"/></td></tr>
  </xsl:if>
 </table>
 
 <hr/>
 <blockquote>
  <xsl:apply-templates select="mime"/>
 </blockquote>
</xsl:template>

</xsl:stylesheet>
