<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    version="1.0">

<xsl:template match="redirect" mode="title">
  <xsl:value-of select="$redirect"/>
</xsl:template>

<xsl:template match="redirect" mode="body">
 <h2>Lurker <xsl:value-of select="$redirect"/></h2>
 <p/><xsl:value-of select="$redirect"/>
 <a href="../{url}.{$ext}">
   ../<xsl:value-of select="url"/>.<xsl:value-of select="$ext"/>
 </a>
</xsl:template>

</xsl:stylesheet>
