<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/1999/xhtml"
    version="1.0">

<xsl:template match="thread" mode="title">
 <xsl:apply-templates select="server" mode="title"/> -
 <xsl:value-of select="summary/subject"/>
</xsl:template>

<xsl:template match="thread" mode="body">
 <p/><xsl:apply-templates select="server" mode="header"/>
 <p/>
 <h2>
  <xsl:value-of select="summary/subject"/>
 </h2>
 
 <p/>
 <table border="0" cellspacing="0" cellpadding="0" class="thread">
    <tr class="row1"><th align="left">Threading</th>
        <th align="left">Author</th>
        <th align="left">Timestamp</th></tr>
    <xsl:apply-templates select="summary"/>
 </table>
</xsl:template>

</xsl:stylesheet>
