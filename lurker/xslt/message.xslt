<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"                    
    xmlns="http://www.w3.org/1999/xhtml"
    version="1.0">

<xsl:output method="html" indent="yes" encoding="UTF-8"
    doctype-system="http://www.w3.org/TR/html4/strict.dtd"
    doctype-public="-//W3C//DTD HTML 4.01//EN"/>

<xsl:template match="br"><br/></xsl:template>

<xsl:template match="email[@address]">
  <xsl:if test="not(position()=1)"><xsl:text>, </xsl:text></xsl:if>
  <a href="mailto:{@address}">
    <xsl:if test="@name">'<xsl:value-of select="@name"/>'</xsl:if>
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
  <xsl:if test="not(position()=1)"><xsl:text>, </xsl:text></xsl:if>
  <a href="{id}.html">
    <xsl:choose>
      <xsl:when test="email/@name"><xsl:value-of select="email/@name"/></xsl:when>
      <xsl:when test="email/@address"><xsl:value-of select="email/@address"/></xsl:when>
      <xsl:otherwise>SomeOne</xsl:otherwise>
    </xsl:choose>'s post on
    <xsl:value-of select="time"/>
  </a>
</xsl:template>

<xsl:template match="mime">
  <xsl:if test="not(position()=1)"><hr/></xsl:if>
  <div align="right">
   <xsl:if test="@name">
     <a href="../attach/{/message/id}@{@id}@{@name}">
     <xsl:value-of select="@name"/> (<xsl:value-of select="@type"/>)
     </a>
   </xsl:if>
   <xsl:if test="not(@name)">
     <a href="../attach/{/message/id}@{@id}.attach">
     (<xsl:value-of select="@type"/>)
     </a>
   </xsl:if>
  </div>
  <xsl:apply-templates/>
</xsl:template>

<!-- Document Root -->
<xsl:template match="/">
 <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
 <head>
  <title>
    Lurker@<xsl:value-of select="/message/server/hostname"/> 
    - <xsl:value-of select="/message/list/email/@name"/>
    - <xsl:value-of select="/message/subject"/>
  </title>
 </head>
 <body>
  <h1>Lurker@<xsl:value-of select="/message/server/hostname"/></h1>
  <h2><xsl:apply-templates select="/message/list/email"/> - 
      <xsl:value-of select="/message/subject"/></h2>

  <table>
    <tr><th align="left">Author:</th>
         <td width="100%"><xsl:choose>
           <xsl:when test="/message/reply-to"><xsl:apply-templates select="/message/reply-to"/></xsl:when>
           <xsl:when test="/message/from"><xsl:apply-templates select="/message/from"/></xsl:when>
           <xsl:when test="/message/sender"><xsl:apply-templates select="/message/sender"/></xsl:when>
         </xsl:choose></td>
         <td><a href="../thread/{/message/thread}.html">thread view</a></td></tr>
    <tr><th align="left">To:</th><td><xsl:apply-templates select="/message/to"/></td>
         <td><a href="../mbox/{/message/id}.txt">
             <xsl:if test="/message/mbox"><xsl:value-of select="/message/mbox"/></xsl:if>
             <xsl:if test="not(/message/mbox)">mbox</xsl:if>
             </a></td></tr>
    <xsl:if test="/message/cc">
     <tr><th align="left">CC:</th><td><xsl:apply-templates select="/message/cc"/></td></tr>
    </xsl:if>
    <tr><th align="left">Date:</th><td><xsl:value-of select="/message/time"/></td></tr>
    <xsl:if test="/message/inreplyto">
      <tr><th aligh="left">In-Reply-To:</th><td><xsl:apply-templates select="/message/inreplyto/summary"/></td></tr>
    </xsl:if>
    <xsl:if test="/message/replies">
      <tr><th aligh="left">Follow-Ups:</th><td><xsl:apply-templates select="/message/replies/summary"/></td></tr>
    </xsl:if>
  </table>

  <hr/>
  <blockquote>
    <xsl:apply-templates select="/message/mime"/>
  </blockquote>
  
 <hr/>
 <p>Administrated by: <xsl:apply-templates select="/message/server/email"/></p>
 </body>
 </html>
</xsl:template>

</xsl:stylesheet>
