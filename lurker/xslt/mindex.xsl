<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    version="1.0">

<xsl:template name="jump">
 <form action="{server/cgi-url}/jump.cgi">
 [ <b><xsl:value-of select="$jumpto"/></b>
  &#160;&#160;
  <input type="hidden" name="doc-url" value="{server/doc-url}"/>
  <input type="hidden" name="format" value="{$ext}"/>
  <input type="hidden" name="list" value="{list/id}"/>

  <xsl:call-template name="date-fields">
   <xsl:with-param name="date" select="row/summary/id"/>
  </xsl:call-template>
  &#160;&#160;
  <input type="submit" value="{$jump}"/> ]
 </form>
</xsl:template>

<xsl:template match="mindex" mode="title">
 <xsl:value-of select="list/email/@name"/> -
 <xsl:value-of select="$mess"/>
 <xsl:text> </xsl:text>
 <xsl:value-of select="offset"/>-<xsl:value-of select="string(number(offset)+count(summary)-1)"/>
</xsl:template>

<xsl:template match="mindex" mode="body">
 <h2>
  <xsl:choose>
   <xsl:when test="prev">
    <a href="{list/id}@{$first-date}.{$ext}"><img alt="&lt;&lt;=" src="../imgs/first.png"/></a>
    <a href="{list/id}@{prev}.{$ext}"><img alt="&lt;-" src="../imgs/prev.png"/></a>
   </xsl:when>
   <xsl:otherwise>
    <img src="../imgs/a.png" alt="..."/>
    <img src="../imgs/a.png" alt=".."/>
   </xsl:otherwise>
  </xsl:choose>
  <xsl:choose>
   <xsl:when test="next">
    <a href="{list/id}@{next}.{$ext}"><img alt="-&gt;" src="../imgs/next.png"/></a>
    <a href="{list/id}@{$last-date}.{$ext}"><img alt="=&gt;&gt;" src="../imgs/last.png"/></a>
   </xsl:when>
   <xsl:otherwise>
    <img src="../imgs/a.png" alt=".."/>
    <img src="../imgs/a.png" alt="..."/>
   </xsl:otherwise>
  </xsl:choose>

  <xsl:text>&#160;&#160;</xsl:text>

  <xsl:apply-templates select="list/email" mode="long"/>
 </h2>
 
 <div id="jumpTo"><xsl:call-template name="jump"/></div>

 <table width="100%" cellpadding="0" cellspacing="0" class="padded" id="mindexOut">
  <tr class="thRow">
   <th align="left"><xsl:value-of select="$subject"/></th>
   <th align="left"><xsl:value-of select="$author"/></th>
   <th align="left"><xsl:value-of select="$date"/> (UTC)</th></tr>
  <xsl:apply-templates select="row"/>
 </table>

 <hr/>

 <form action="{server/cgi-url}/keyword.cgi" id="smSearch">
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
