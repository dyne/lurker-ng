<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    version="1.0">

<xsl:template match="error" mode="title">
 <xsl:value-of select="title"/>
</xsl:template>

<xsl:template match="error" mode="body">
 <h2><xsl:value-of select="title"/>:</h2>
 <p/><xsl:value-of select="message"/>
 <p/><xsl:value-of select="detail"/>
</xsl:template>

</xsl:stylesheet>
