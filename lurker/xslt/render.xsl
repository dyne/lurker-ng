<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/1999/xhtml"
    version="1.0">

<xsl:import href="lang.xsl"/>
<xsl:import href="common.xsl"/>
<xsl:import href="error.xsl"/>
<xsl:import href="redirect.xsl"/>

<xsl:import href="splash.xsl"/>
<xsl:import href="message.xsl"/>
<xsl:import href="mindex.xsl"/>
<xsl:import href="search.xsl"/>
<xsl:import href="thread.xsl"/>

<!--- The default is to render to xml and use browser -->
<xsl:variable name="ext">xml</xsl:variable>

<xsl:output method="html" indent="no" encoding="UTF-8"
    doctype-system="http://www.w3.org/TR/html4/strict.dtd"
    doctype-public="-//W3C//DTD HTML 4.0 Transitional//EN"/>

<!-- Document Root -->
<xsl:template match="/">
 <html xmlns="http://www.w3.org/1999/xhtml" lang="{$lang}">
  <head>
   <xsl:if test="/redirect/url">
    <xsl:element name="meta">
     <xsl:attribute name="content">
      <xsl:text>0;URL=../</xsl:text>
      <xsl:value-of select="/redirect/url"/>
      <xsl:text>.</xsl:text>
      <xsl:value-of select="$ext"/>
     </xsl:attribute>
     <xsl:attribute name="http-equiv">
      <xsl:text>Refresh</xsl:text>
     </xsl:attribute>
    </xsl:element>
   </xsl:if>
   <xsl:if test="/*/server/expires">
    <meta content="{/*/server/expires}" http-equiv="Expires"/>
   </xsl:if>
   <link rel="stylesheet" href="../fmt/default.css" type="text/css"/>
   <title>
    <xsl:apply-templates select="/*/server" mode="title"/> -
    <xsl:apply-templates mode="title"/>
   </title>
  </head>
  <body>
   <!-- Server Header -->
   <h1><xsl:apply-templates select="*/server" mode="header"/></h1>

   <!-- Request Body -->
   <xsl:apply-templates mode="body"/>

   <!--  Server Footer -->
   <hr/>
   <div id="footer">
    <xsl:apply-templates select="*/server" mode="footer"/>
   </div>
  </body>
 </html>
</xsl:template>

</xsl:stylesheet>
