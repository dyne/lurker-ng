<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    version="1.0">

<xsl:template match="list" mode="select">
 <option value="{id}">
  <xsl:value-of select="email/@name"/>
  <xsl:if test="email/@address">
   <xsl:text>&#160;&lt;</xsl:text>
   <xsl:value-of select="email/@address"/>
   <xsl:text>&gt;</xsl:text>
  </xsl:if>
 </option>
</xsl:template>

<xsl:template match="list">
 <a href="../mindex/{id}@{$last-date}.{$ext}">
  <xsl:value-of select="email/@name"/>
 </a>
</xsl:template>

<xsl:template name="format">
 <xsl:param name="col1"/>
 <xsl:param name="col2"/>
 <xsl:param name="col3"/>
 <xsl:param name="col4"/>

 <xsl:if test="count($col1) &gt; 0 or count($col2) &gt; 0 or count($col3) &gt; 0 or count($col4) &gt; 0">
  <xsl:text>
  </xsl:text>
  <tr>
   <td nowrap="NOWRAP">
    <xsl:choose>
     <xsl:when test="count($col1) &gt; 0">
      <xsl:apply-templates select="$col1[position() = 1]"/>
     </xsl:when>
     <xsl:otherwise>&#160;</xsl:otherwise>
    </xsl:choose>
   </td>
   <td nowrap="NOWRAP">
    <xsl:choose>
     <xsl:when test="count($col2) &gt; 0">
      <xsl:apply-templates select="$col2[position() = 1]"/>
     </xsl:when>
     <xsl:otherwise>&#160;</xsl:otherwise>
    </xsl:choose>
   </td>
   <td nowrap="NOWRAP">
    <xsl:choose>
     <xsl:when test="count($col3) &gt; 0">
      <xsl:apply-templates select="$col3[position() = 1]"/>
     </xsl:when>
     <xsl:otherwise>&#160;</xsl:otherwise>
    </xsl:choose>
   </td>
   <td nowrap="NOWRAP">
    <xsl:choose>
     <xsl:when test="count($col4) &gt; 0">
      <xsl:apply-templates select="$col4[position() = 1]"/>
     </xsl:when>
     <xsl:otherwise>&#160;</xsl:otherwise>
    </xsl:choose>
   </td>
  </tr>
  <xsl:call-template name="format">
   <xsl:with-param name="col1" select="$col1[position() &gt; 1]"/>
   <xsl:with-param name="col2" select="$col2[position() &gt; 1]"/>
   <xsl:with-param name="col3" select="$col3[position() &gt; 1]"/>
   <xsl:with-param name="col4" select="$col4[position() &gt; 1]"/>
  </xsl:call-template>
 </xsl:if>
</xsl:template>

<xsl:template match="splash" mode="title">
 <xsl:value-of select="$splash"/>
</xsl:template>

<xsl:template match="splash" mode="body">
 <h2><xsl:value-of select="$lists"/></h2>
 <div id="listBlock">
 <table id="listTable" width="100%">
  <xsl:variable name="size1" select="floor((count(list)+3) div 4)"/>
  <xsl:variable name="size2" select="floor((count(list)+2) div 4)"/>
  <xsl:variable name="size3" select="floor((count(list)+1) div 4)"/>
  <xsl:variable name="size4" select="floor((count(list)+0) div 4)"/>
  
  <xsl:variable name="off1" select="1"/>
  <xsl:variable name="off2" select="$off1+$size1"/>
  <xsl:variable name="off3" select="$off2+$size2"/>
  <xsl:variable name="off4" select="$off3+$size3"/>
  <xsl:variable name="off5" select="$off4+$size4"/>

  <tbody>
   <xsl:call-template name="format">
    <xsl:with-param name="col1" select="list[position() &gt;= $off1 and position() &lt; $off2]"/>
    <xsl:with-param name="col2" select="list[position() &gt;= $off2 and position() &lt; $off3]"/>
    <xsl:with-param name="col3" select="list[position() &gt;= $off3 and position() &lt; $off4]"/>
    <xsl:with-param name="col4" select="list[position() &gt;= $off4 and position() &lt; $off5]"/>
   </xsl:call-template>
  </tbody>
 </table>
 </div>

 <br/><br/><hr/>
 <h2><xsl:value-of select="$titlesearch"/></h2>

 <table width="100%"><tr><td width="50%">&#160;</td><td>
 <form action="{server/cgi-url}/keyword.cgi">
  <input type="hidden" name="doc-url" value="{server/doc-url}"/>
  <input type="hidden" name="format" value="{$ext}"/>
  <input type="text" name="query" class="longtext"/>
  <input type="submit" name="submit" value="{$search}"/>
 <hr id="searchHr"/>
  <table id="searchTable">
   <tr><th><xsl:value-of select="$subject"/></th>
       <td><input type="text" name="subject" class="longtext"/></td></tr>
   <tr><th><xsl:value-of select="$author"/></th>
       <td><input type="text" name="author" class="longtext"/></td></tr>
   <tr>
    <th><xsl:value-of select="$appearinlist"/></th>
    <td>
     <select name="list" id="listdrop">
      <option value=""><xsl:value-of select="$all-li"/></option>
      <xsl:apply-templates mode="select" select="list"/>
     </select>
    </td>
   </tr>
   <tr>
    <th><xsl:value-of select="$startdate"/></th>
    <td>
     <xsl:call-template name="date-fields">
      <xsl:with-param name="date" select="$jump-date"/>
     </xsl:call-template>
    </td>
   </tr>
  </table>
 </form>
 </td><td width="50%">&#160;</td></tr></table>

 <br/><hr/>

 <h2><xsl:value-of select="$info"/></h2>

 <dl><xsl:apply-templates select="list" mode="longdesc"/></dl>

</xsl:template>

<xsl:template match="list" mode="longdesc">
 <dt>
  <xsl:value-of select="email/@name"/>
  <xsl:if test="email/@address">
   <xsl:text>&#160;</xsl:text>
   <xsl:element name="a">
    <xsl:attribute name="href">
     <xsl:text>mailto:</xsl:text>
     <xsl:apply-templates select="email" mode="mailto"/>
    </xsl:attribute>

    <xsl:text>(</xsl:text>
    <xsl:value-of select="email/@address"/>
    <xsl:text>)</xsl:text>
   </xsl:element>
  </xsl:if>
 </dt>
 <dd>
  [ <a href="../mindex/{id}@{$last-date}.{$ext}">
     <xsl:value-of select="$mess"/>
    </a>
  ]
  <xsl:if test="link">
   [ <a href="{link}">
      <xsl:value-of select="$homepage"/>
     </a>
   ]
  </xsl:if>

  <xsl:text> -- </xsl:text>

  <xsl:if test="description"><span class="desc"><xsl:value-of select="description"/></span></xsl:if>
  <xsl:if test="not(description)"><span class="desc"><xsl:value-of select="$nodesc"/></span></xsl:if>
 </dd>
</xsl:template>

</xsl:stylesheet>
