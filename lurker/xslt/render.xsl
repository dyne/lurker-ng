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
 <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="{$lang}" lang="{$lang}">
  <head>
   <link rel="stylesheet" href="../fmt/default.css" type="text/css"/>
   <title>
    <xsl:apply-templates select="*/server" mode="title"/> -
    <xsl:apply-templates mode="title"/>
   </title>
   <xsl:if test="/redirect/url">
    <meta content="0;URL=../{/redirect/url}.{$ext}" http-equiv="Refresh"/>
   </xsl:if>
  </head>
  <body>
   <!-- Server Header -->
   <p/><xsl:apply-templates select="*/server" mode="header"/>

   <!-- Request Body -->
   <xsl:apply-templates mode="body"/>

   <!--  Server Footer -->
   <p/><hr/>
   <div class="center">
    <xsl:apply-templates select="*/server" mode="footer"/>
   </div>
  </body>
 </html>
</xsl:template>

</xsl:stylesheet>
