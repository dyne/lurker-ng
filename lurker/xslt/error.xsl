<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/1999/xhtml"
    version="1.0">

<xsl:template match="error" mode="title">
 <xsl:apply-templates select="server" mode="title"/> -
 <xsl:value-of select="title"/>
</xsl:template>

<xsl:template match="error" mode="body">
 <p/><xsl:apply-templates select="server" mode="header"/>
 <p/><h2><xsl:value-of select="title"/>:</h2>
 <p/><xsl:value-of select="message"/>
 <p/><xsl:value-of select="detail"/>
 <hr/>
 <p/><xsl:apply-templates select="server" mode="footer"/>
</xsl:template>

</xsl:stylesheet>
