<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/1999/xhtml"
    version="1.0">

<xsl:template match="thread" mode="title">
 <xsl:value-of select="summary/subject"/>
</xsl:template>

<xsl:template match="thread" mode="body">
 <p/>
 <h2>
  <xsl:value-of select="summary/subject"/>
 </h2>
 
 <p/>
 <table width="100%" border="0" cellspacing="0" cellpadding="0" class="thread">
  <tr class="row1">
   <th align="left"><xsl:value-of select="$threading"/></th>
   <th align="left"><xsl:value-of select="$author"/></th>
   <th align="left"><xsl:value-of select="$date"/></th>
  </tr>
  <xsl:apply-templates select="summary"/>
 </table>

 <hr/>
 <p/>
 
 <form action="../lurker-search.cgi">
  <input type="hidden" name="format" value="{$ext}"/>
  <input type="text"   name="query"  value="th:{id} " size="50"/>
  <input type="submit" name="submit" value="{$search}!"/>
 </form> 
</xsl:template>

</xsl:stylesheet>
