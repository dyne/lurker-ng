<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/1999/xhtml"
    version="1.0">

<xsl:template match="search" mode="title">
 <xsl:value-of select="$search"/>: <xsl:value-of select="query"/>
</xsl:template>

<xsl:template match="search" mode="body">
 <h2>
  <xsl:if test="prev">
   <xsl:element name="a">
    <xsl:attribute name="href">
     <xsl:text>0%20</xsl:text>
     <xsl:call-template name="my-escape-uri">
      <xsl:with-param name="str" select="queryurl"/>
     </xsl:call-template>
     <xsl:text>.</xsl:text>
     <xsl:value-of select="$ext"/>
    </xsl:attribute>
    
    <img border="0" src="../imgs/first.png" alt="&lt;&lt;="/>
   </xsl:element>
   
   <xsl:element name="a">
    <xsl:attribute name="href">
     <xsl:value-of select="prev"/>
     <xsl:text>%20</xsl:text>
     <xsl:call-template name="my-escape-uri">
      <xsl:with-param name="str" select="queryurl"/>
     </xsl:call-template>
     <xsl:text>.</xsl:text>
     <xsl:value-of select="$ext"/>
    </xsl:attribute>
    
    <img border="0" src="../imgs/prev.png" alt="&lt;-"/>
   </xsl:element>
  </xsl:if>
  <xsl:if test="not(prev)">
   <img src="../imgs/a.png" alt="..."/>
   <img src="../imgs/a.png" alt=".."/>
  </xsl:if>
  <xsl:if test="next">
   <xsl:element name="a">
    <xsl:attribute name="href">
     <xsl:value-of select="next"/>
     <xsl:text>%20</xsl:text>
     <xsl:call-template name="my-escape-uri">
      <xsl:with-param name="str" select="queryurl"/>
     </xsl:call-template>
     <xsl:text>.</xsl:text>
     <xsl:value-of select="$ext"/>
    </xsl:attribute>
    
    <img border="0" src="../imgs/next.png" alt="-&gt;"/>
   </xsl:element>
  </xsl:if>
  <xsl:if test="not(next)">
   <img src="../imgs/a.png" alt=".."/>
  </xsl:if>

  <xsl:text disable-output-escaping="yes">&amp;nbsp;&amp;nbsp;&amp;nbsp;</xsl:text>
  
  <xsl:value-of select="$search"/> - <xsl:value-of select="$mess"/>
  <xsl:text> </xsl:text>
  <xsl:value-of select="offset"/>-<xsl:value-of select="string(number(offset)+count(summary))"/>
  / ~<xsl:value-of select="hits"/>
 </h2>
 
 <div id="smSearch">
  <form action="../lurker-search.cgi">
   <input type="hidden" name="format" value="{$ext}"/>
   <input type="text"   name="query"  value="{query}" class="longtext"/>
   <input type="submit" name="submit" value="{$search}!"/>
  </form>
 </div>

 <table width="100%" cellspacing="0" cellpadding="0" class="padded" id="searchOut">
  <tr class="thRow">
   <th align="left"><xsl:value-of select="$subject"/></th>
   <th align="left"><xsl:value-of select="$author"/></th>
   <th align="left"><xsl:value-of select="$date"/></th></tr>
  <xsl:apply-templates select="summary"/>
 </table>

 <br/>

</xsl:template>

</xsl:stylesheet>
