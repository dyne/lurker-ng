<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/1999/xhtml"
    version="1.0">


<!-- Truncate long strings -->

<xsl:template name="truncate">
 <xsl:param name="length"/>
 <xsl:param name="string"/>
 <xsl:choose>
  <xsl:when test="string-length($string) &gt; $length">
   <xsl:value-of select="substring($string,1,$length - 2)"/>
   <xsl:text> ...</xsl:text>
  </xsl:when>
  <xsl:otherwise>
   <xsl:value-of select="$string"/>
  </xsl:otherwise>
 </xsl:choose>
</xsl:template>

<!-- Output option-ranges -->

<xsl:template name="option-range">
 <xsl:param name="start"/>
 <xsl:param name="last"/>
 <xsl:param name="select"/>
 <xsl:element name="option">
  <xsl:attribute name="value"><xsl:value-of select="$start"/></xsl:attribute>
  <xsl:if test="$start = $select">
   <xsl:attribute name="selected">yes</xsl:attribute>
  </xsl:if>
  <xsl:value-of select="$start"/>
 </xsl:element>
 <xsl:if test="$start &lt; $last">
  <xsl:call-template name="option-range">
   <xsl:with-param name="start"> <xsl:value-of select="$start+1"/></xsl:with-param>
   <xsl:with-param name="last"> <xsl:value-of select="$last"/></xsl:with-param>
   <xsl:with-param name="select"><xsl:value-of select="$select"/> </xsl:with-param>
  </xsl:call-template>
 </xsl:if>
</xsl:template>

<!-- Escape URIs -->

<xsl:variable name="uri-input">The@dog-z went/_&#127;_%ab%zu%c</xsl:variable>
<xsl:variable name="uri-output">The%40dog-z%20went%2F_%7F_%ab%25zu%25c</xsl:variable>
<xsl:variable name="have-escape-uri" select="false()"/>
<!-- "escape-uri($uri-input, true()) = $uri-output" -->

<!-- According to the RFC, non-ascii chars will be utf-8 encoded and escaped
     with %s by the xslt-engine when in a 'uri' attribute or by the browser
     if the xlst-engine doesn't. This is ok, but not enough since we still
     won't have working RFC822 (email) Froms! since they need =s. 
     However, as there is nothing I can do about this, I will just hope for
     the best if an xslt engine doesn't have uri-escape. -->
<xsl:variable name="ascii-charset"> !&quot;#$%&amp;&apos;()*+,-./0123456789:;&lt;=&gt;?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~&#127;</xsl:variable>
<xsl:variable name="uri-ok">-_.!~*&apos;()0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz</xsl:variable>
<xsl:variable name="hex">0123456789ABCDEFabcdef</xsl:variable>

<xsl:template name="do-escape-uri">
 <xsl:param name="str"/>
 <xsl:param name="allow-utf8"/>

 <xsl:if test="$str">
  <xsl:variable name="first-char" select="substring($str,1,1)"/>
  <xsl:choose>
   <xsl:when test="$first-char = '%' and string-length($str) &gt;= 3 and contains($hex, substring($str,2,1)) and contains($hex, substring($str,3,1))">
    <!-- The percent char is ok IF it followed by a valid hex pair -->
    <xsl:value-of select="$first-char"/>
   </xsl:when>
   <xsl:when test="contains($uri-ok, $first-char)">
    <!-- This char is ok inside urls -->
    <xsl:value-of select="$first-char"/>
   </xsl:when>
   <xsl:when test="not(contains($ascii-charset, $first-char))">
    <!-- Non-ascii output raw based on utf8 allowed or not -->
    <xsl:choose>
     <xsl:when test="$allow-utf8">
      <xsl:value-of select="$first-char"/>
     </xsl:when>
     <xsl:otherwise>
      <xsl:text>%3F</xsl:text>
     </xsl:otherwise>
    </xsl:choose>
   </xsl:when>
   <xsl:otherwise>
    <!-- URL escape this char -->
    <xsl:variable name="ascii-value" select="string-length(substring-before($ascii-charset,$first-char)) + 32"/>
    <xsl:text>%</xsl:text>
    <xsl:value-of select="substring($hex,floor($ascii-value div 16) + 1,1)"/>
    <xsl:value-of select="substring($hex,$ascii-value mod 16 + 1,1)"/>
   </xsl:otherwise>
  </xsl:choose>
  
  <xsl:call-template name="do-escape-uri">
   <xsl:with-param name="str" select="substring($str,2)"/>
   <xsl:with-param name="allow-utf8" select="$allow-utf8"/>
  </xsl:call-template>
 </xsl:if>
</xsl:template>

<xsl:template name="my-escape-uri">
 <xsl:param name="str"/>
 <xsl:param name="allow-utf8"/>
 
 <xsl:choose>
  <xsl:when test="$have-escape-uri">
<!--   <xsl:value-of select="escape-uri($str, true())"/> -->
  </xsl:when>
  <xsl:otherwise>
   <xsl:call-template name="do-escape-uri">
    <xsl:with-param name="str" select="$str"/>
    <xsl:with-param name="allow-utf8" select="$allow-utf8"/>
   </xsl:call-template>
  </xsl:otherwise>
 </xsl:choose>
</xsl:template>

<!-- Encode email headers with charset wrappers -->

<xsl:variable name="subject-allow"> &quot;#$%&amp;+,/:;&lt;=&gt;?@[\]^{|}</xsl:variable>
<xsl:variable name="email-allow"> #$%&amp;+/:[]^{|}</xsl:variable>

<xsl:template name="email-header">
 <xsl:param name="charset"/>
 <xsl:param name="str"/>
 
 <xsl:variable name="pure">
  <xsl:call-template name="my-escape-uri">
   <xsl:with-param name="str" select="translate($str, $charset, '')"/>
  </xsl:call-template>
 </xsl:variable>
 
 <xsl:choose>
  <xsl:when test="contains($pure, '%')">
   <xsl:variable name="uri">
    <xsl:call-template name="my-escape-uri">
     <xsl:with-param name="str" select="$str"/>
    </xsl:call-template>
   </xsl:variable>
 
   <xsl:text>=?utf-8?Q?</xsl:text>
   <xsl:value-of select="translate($uri, '%', '=')"/>
   <xsl:text>?=</xsl:text>
  </xsl:when>
  
  <xsl:otherwise>
   <xsl:value-of select="$str"/>
  </xsl:otherwise>
 </xsl:choose>
</xsl:template>

<!-- Format email address -->

<xsl:template name="email-simple" match="email" mode="simple">
 <xsl:choose>
  <xsl:when test="@name"><xsl:value-of select="@name"/></xsl:when>
  <xsl:when test="@address"><xsl:value-of select="@address"/></xsl:when>
  <xsl:otherwise><xsl:value-of select="$unknown"/></xsl:otherwise>
 </xsl:choose>
</xsl:template>

<xsl:template name="email-mailto" match="email" mode="mailto">
 <xsl:if test="@name">
  <xsl:call-template name="my-escape-uri">
   <xsl:with-param name="str">
    <xsl:call-template name="email-header">
     <xsl:with-param name="str" select="@name"/>
     <xsl:with-param name="charset" select="$email-allow"/>
    </xsl:call-template>
   </xsl:with-param>
  </xsl:call-template>
  
  <xsl:text>%20</xsl:text>
 </xsl:if>
 
 <xsl:text>%3C</xsl:text>
 <xsl:call-template name="my-escape-uri">
  <xsl:with-param name="str" select="@address"/>
 </xsl:call-template>
 <xsl:text>%3E</xsl:text>
</xsl:template>

<xsl:template name="email-cut" match="email" mode="cut">
 <xsl:call-template name="truncate">
  <xsl:with-param name="length">20</xsl:with-param>
  <xsl:with-param name="string"><xsl:call-template name="email-simple"/></xsl:with-param>
 </xsl:call-template>
</xsl:template>

<xsl:template match="email[@address]">
 <xsl:element name="a">
  <xsl:attribute name="href">
   <xsl:text>mailto:</xsl:text>
   <xsl:call-template name="email-mailto"/>
  </xsl:attribute>
  <xsl:call-template name="email-cut"/>
 </xsl:element>
</xsl:template>

<xsl:template match="email">
  <xsl:call-template name="email-cut"/>
</xsl:template>

<xsl:template match="email[@address]" mode="list">
 <xsl:if test="not(position()=1)"><xsl:text>, </xsl:text><br/></xsl:if>

 <xsl:element name="a">
  <xsl:attribute name="href">
   <xsl:text>mailto:</xsl:text>
   <xsl:call-template name="email-mailto"/>
  </xsl:attribute>
  <xsl:call-template name="email-simple"/>
 </xsl:element>
</xsl:template>

<xsl:template match="email" mode="list">
 <xsl:if test="not(position()=1)"><xsl:text>, </xsl:text><br/></xsl:if>
 <xsl:call-template name="email-simple"/>
</xsl:template>


<!-- Format summary lists -->

<xsl:template name="msg-thread">
 <xsl:param name="alt" select="$malt"/>
 
 <xsl:element name="a">
  <xsl:attribute name="class">thm</xsl:attribute>
  <xsl:attribute name="href">
   <xsl:text>../message/</xsl:text>
   <xsl:call-template name="my-escape-uri">
    <xsl:with-param name="str">
     <xsl:choose>
      <xsl:when test="summary">
       <xsl:value-of select="summary/mid"/>
      </xsl:when>
      <xsl:when test="ancestor::snippet">
       <xsl:value-of select="descendant::snippet/mid"/>
      </xsl:when>
      <xsl:when test="ancestor::summary">
       <xsl:value-of select="ancestor::summary/mid"/>
      </xsl:when>
     </xsl:choose>
    </xsl:with-param>
   </xsl:call-template>
   <xsl:text>.</xsl:text>
   <xsl:value-of select="$ext"/>
  </xsl:attribute>
  
  <xsl:element name="img">
   <xsl:attribute name="alt">
    <xsl:value-of select="$alt"/>
   </xsl:attribute>
   
   <xsl:attribute name="title">
    <xsl:choose>
     <xsl:when test="summary">
      <xsl:apply-templates select="summary" mode="post"/>
     </xsl:when>
     <xsl:when test="ancestor::snippet">
      <xsl:apply-templates select="descendant::snippet" mode="post"/>
     </xsl:when>
     <xsl:when test="ancestor::summary">
      <xsl:apply-templates select="ancestor::summary" mode="post"/>
     </xsl:when>
    </xsl:choose>
   </xsl:attribute>
   
   <xsl:attribute name="src">
    <xsl:text>../imgs/</xsl:text>
    <xsl:value-of select="local-name(.)"/>
    <xsl:text>.png</xsl:text>
   </xsl:attribute>
   
   <xsl:attribute name="class">
    <xsl:choose>
     <xsl:when test="@selected">selected</xsl:when>
     <xsl:when test="@drift">drift</xsl:when>
     <xsl:otherwise>normal</xsl:otherwise>
    </xsl:choose>
   </xsl:attribute>
  </xsl:element>
 </xsl:element>
</xsl:template>

<xsl:template match="a"><img alt="." src="../imgs/a.png"/></xsl:template>
<xsl:template match="b"><img alt="|" src="../imgs/b.png"/></xsl:template>
<xsl:template match="c"><img alt="-" src="../imgs/c.png"/></xsl:template>
<xsl:template match="d"><img alt="\" src="../imgs/d.png"/></xsl:template>
<xsl:template match="e"><img alt="+" src="../imgs/e.png"/></xsl:template>
<xsl:template match="f"><xsl:call-template name="msg-thread"/></xsl:template>
<xsl:template match="g"><xsl:call-template name="msg-thread"/></xsl:template>
<xsl:template match="h"><xsl:call-template name="msg-thread"/></xsl:template>
<xsl:template match="i"><xsl:call-template name="msg-thread"/></xsl:template>
<xsl:template match="j"><xsl:call-template name="msg-thread"/></xsl:template>
<xsl:template match="k"><xsl:call-template name="msg-thread"/></xsl:template>
<xsl:template match="br"><xsl:if test="not(ancestor::art)"><br/></xsl:if></xsl:template>

<xsl:template match="summary">
 <xsl:element name="tr">
  <xsl:if test="(position() mod 2) = 0">
   <xsl:attribute name="class">row1</xsl:attribute>
  </xsl:if>
  <xsl:if test="(position() mod 2) = 1">
   <xsl:attribute name="class">row2</xsl:attribute>
  </xsl:if>
  <td nowrap="1">
   <a name="{id}"/>
   <xsl:if test="draw"><xsl:apply-templates select="draw"/></xsl:if>
   <xsl:if test="not(draw)">
    <a href="../thread/{thread}.{$ext}">
     <img src="../imgs/tree.png" alt="{$threading}"/>
    </a>
    <xsl:text disable-output-escaping="yes">&amp;nbsp;</xsl:text>
    <xsl:element name="a">
     <xsl:attribute name="href">
      <xsl:text>../message/</xsl:text>
      <xsl:call-template name="my-escape-uri">
       <xsl:with-param name="str" select="mid"/>
      </xsl:call-template>
      <xsl:text>.</xsl:text>
      <xsl:value-of select="$ext"/>
     </xsl:attribute>
     
     <xsl:call-template name="truncate">
      <xsl:with-param name="length">50</xsl:with-param>
      <xsl:with-param name="string"><xsl:value-of select="subject"/></xsl:with-param>
     </xsl:call-template>
    </xsl:element>
   </xsl:if>
  </td>
  <td nowrap="1"><xsl:apply-templates select="email"/></td>
  <td nowrap="1"><xsl:value-of select="time"/></td>
 </xsl:element>
</xsl:template>


<!-- Format list of threading summary lists -->

<xsl:template name="summary-post">
 <xsl:value-of select="$post1"/>
 <xsl:apply-templates select="email" mode="simple"/>
 <xsl:value-of select="$post2"/>
 <xsl:text> </xsl:text>
 <xsl:value-of select="time"/>
</xsl:template>

<xsl:template match="summary" mode="post">
 <xsl:call-template name="summary-post"/>
</xsl:template>

<xsl:template match="summary" mode="list">
 <xsl:if test="not(position()=1)"><xsl:text>, </xsl:text><br/></xsl:if>
 <xsl:element name="a">
  <xsl:attribute name="href">
   <xsl:text>../message/</xsl:text>
   <xsl:call-template name="my-escape-uri">
    <xsl:with-param name="str" select="mid"/>
   </xsl:call-template>
   <xsl:text>.</xsl:text>
   <xsl:value-of select="$ext"/>
  </xsl:attribute>
  
  <xsl:attribute name="class">
   <xsl:if test="thread-head">
    <xsl:text>drift</xsl:text>
   </xsl:if>
  </xsl:attribute>
  
  <xsl:call-template name="summary-post"/>
  <xsl:if test="drift"> (drifted)</xsl:if>
 </xsl:element>
</xsl:template>


<!-- Format server information segments -->

<xsl:template match="server" mode="title">
  Lurker@<xsl:value-of select="hostname"/> 
</xsl:template>

<xsl:template match="server" mode="header">
 <a href="../splash/index.{$ext}">
  Lurker@<xsl:value-of select="hostname"/>
 </a>
</xsl:template>

<xsl:template match="server" mode="footer">
 Lurker <xsl:value-of select="version"/> - 
 <xsl:value-of select="$adminby"/> <xsl:apply-templates select="email"/>
</xsl:template>

</xsl:stylesheet>
