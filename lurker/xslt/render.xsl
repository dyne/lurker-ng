<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"                    
    xmlns="http://www.w3.org/1999/xhtml"
    version="1.0">

<xsl:import href="lang.xsl"/>
<xsl:import href="common.xsl"/>
<xsl:import href="error.xsl"/>

<xsl:import href="splash.xsl"/>
<xsl:import href="message.xsl"/>

<!--- The default is to render to xml and use browser -->
<xsl:variable name="ext">xml</xsl:variable>

<xsl:output method="html" indent="yes" encoding="UTF-8"
    doctype-system="http://www.w3.org/TR/html4/strict.dtd"
    doctype-public="-//W3C//DTD HTML 4.01//EN"/>

<!-- Document Root -->
<xsl:template match="/">
 <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="{$lang}" lang="{$lang}">
  <head>
   <link href="../fmt/default.css" type="text/css"/>
   <title><xsl:apply-templates mode="title"/></title>
  </head>
  <body>
   <xsl:apply-templates mode="body"/>
  </body>
 </html>
</xsl:template>

</xsl:stylesheet>
