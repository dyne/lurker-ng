<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    version="1.0">

<xsl:template match="thread" mode="title">
 <xsl:value-of select="summary/subject"/>
</xsl:template>

<xsl:template match="thread" mode="body">
 <h2>
  <xsl:value-of select="row/summary/subject"/>
 </h2>
 
 <table width="100%" cellspacing="0" cellpadding="0" id="threadOut">
  <tr class="thRow">
   <th align="left"><xsl:value-of select="$threading"/></th>
   <th align="left"><xsl:value-of select="$author"/></th>
   <th align="left"><xsl:value-of select="$date"/> (UTC)</th>
  </tr>
  <xsl:apply-templates select="row"/>
 </table>

 <hr/>
 
 <form action="{server/cgi-url}/keyword.cgi" id="smSearch">
  <input type="hidden" name="doc-url" value="{server/doc-url}"/>
  <input type="hidden" name="format"  value="{$ext}"/>
  <input type="text"   name="query"   value="th:{hash} " class="longtext"/>
  <input type="submit" name="submit"  value="{$search}"/>
  <br/>
  <xsl:value-of select="$useTh1"/>
  <b>th:<xsl:value-of select="hash"/></b>
  <xsl:value-of select="$useTh2"/>
 </form> 
</xsl:template>

</xsl:stylesheet>
