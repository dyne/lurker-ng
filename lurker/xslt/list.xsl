<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    version="1.0">

<xsl:template match="day">
 <img src="../imgs/bar.png" height="{number(.)*4+1}" width="10"/>
</xsl:template>

<xsl:template match="list" mode="title">
 <xsl:value-of select="list/email/@name"/>
</xsl:template>

<xsl:template match="row" mode="newtopic">
 <xsl:element name="tr">
  <xsl:attribute name="class">
   <xsl:choose>
    <xsl:when test="@selected">lit</xsl:when>
    <xsl:when test="(position() mod 2) = 0">row1</xsl:when>
    <xsl:otherwise>row2</xsl:otherwise>
   </xsl:choose>
  </xsl:attribute>

  <td nowrap="NOWRAP">
   <a name="{id}"/>
   
   <a href="../thread/{summary/id}.{$ext}">
    <xsl:call-template name="truncate"> 
     <xsl:with-param name="length">50</xsl:with-param>
     <xsl:with-param name="string"><xsl:value-of select="summary/subject"/></xsl:with-param>
    </xsl:call-template>
   </a>
  </td>
  
  <td nowrap="NOWRAP"><xsl:apply-templates select="summary/email"/></td>
  <td nowrap="NOWRAP" class="chart" valign="bottom"><xsl:apply-templates select="day"/></td>
 </xsl:element>
</xsl:template>

<xsl:template match="list" mode="body">
 <h2>
  <xsl:apply-templates select="list/email" mode="long"/>
 </h2>
 
 <table width="100%" cellpadding="0" cellspacing="0" class="padded" id="mindexOut">
  <tr class="thRow">
   <th align="left"><xsl:value-of select="$subject"/></th>
   <th align="left"><xsl:value-of select="$author"/></th>
   <th align="left"><xsl:value-of select="$activity"/></th></tr>
  <xsl:apply-templates mode="newtopic" select="row"/>
 </table>

 <hr/>

 <div id="jumpTo"><xsl:call-template name="jump"/></div>

 <form action="{server/cgi-url}/keyword.cgi" id="smSearch" accept-charset="UTF-8">
  <input type="hidden" name="doc-url" value="{server/doc-url}"/>
  <input type="hidden" name="format"  value="{$ext}"/>
  <input type="text"   name="query"   value="ml:{list/id} " class="longtext"/>
  <input type="submit" name="submit"  value="{$search}"/><br/>

  <xsl:value-of select="$useMl1"/>
  <b>ml:<xsl:value-of select="list/id"/></b>
  <xsl:value-of select="$useMl2"/>
 </form>
</xsl:template>

</xsl:stylesheet>
