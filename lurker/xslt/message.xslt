<?xml version="1.0" encoding="ISO-8859-1" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"                    
    xmlns="http://www.w3.org/1999/xhtml"
    version="1.0">

<xsl:output method="html" indent="yes" encoding="ISO-8859-1"
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

<!-- Document Root -->
<xsl:template match="/">
 <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
 <head>
  <title>
    Lurker@<xsl:value-of select="/message/list/name"/> -
    <xsl:value-of select="/message/subject"/>
  </title>
 </head>
 <body>
  <h1>Lurker@<xsl:value-of select="/message/list/name"/>
      <xsl:if test="/message/list/address">
        <a href="mailto:{/message/list/address}">
         <xsl:value-of select="/message/list/address"/>
        </a>
      </xsl:if>
  </h1>

  <h2><xsl:value-of select="/message/subject"/></h2>

  <table>
    <xsl:if test="/message/from">
     <tr><th align="left">From:</th><td><xsl:apply-templates select="/message/from"/></td></tr>
    </xsl:if>
    <xsl:if test="/message/sender">
     <tr><th align="left">Sender:</th><td><xsl:apply-templates select="/message/sender"/></td></tr>
    </xsl:if>
    <xsl:if test="/message/reply-to">
     <tr><th align="left">Reply-To:</th><td><xsl:apply-templates select="/message/reply-to"/></td></tr>
    </xsl:if>
    <xsl:if test="/message/to">
     <tr><th align="left">To:</th><td><xsl:apply-templates select="/message/to"/></td></tr>
    </xsl:if>
    <xsl:if test="/message/cc">
     <tr><th align="left">CC:</th><td><xsl:apply-templates select="/message/cc"/></td></tr>
    </xsl:if>
    <tr><th align="left">Date:</th><td><xsl:value-of select="/message/time"/></td></tr>
  </table>

  <hr/>
  <blockquote>
    <xsl:apply-templates select="/message/body"/>
  </blockquote>
  <hr/>
  
  <a href="{/message/id}.mbox">Raw</a>
  <xsl:text> </xsl:text>
  <a href="../thread/{/message/thread}.html">Thread</a>

 </body>
 </html>
</xsl:template>

</xsl:stylesheet>
