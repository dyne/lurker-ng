<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/1999/xhtml"
    version="1.0">

<xsl:template match="redirect" mode="title">
  Redirect
</xsl:template>

<xsl:template match="redirect" mode="body">
 <p/><h2>Lurker redirect</h2>
 <p/>Your browser should load this location: 
 <a href="../{url}.{$ext}">
   ../<xsl:value-of select="url"/>.<xsl:value-of select="$ext"/>
 </a>
</xsl:template>

</xsl:stylesheet>
